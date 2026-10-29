#!/usr/bin/env python3
"""Помощник для UI-тестов Flipper Zero по USB: снимки экрана, нажатия, файлы на SD.

Зависимости (в отдельном venv):
    python3.11 -m venv ~/.venvs/flipper-rpc
    ~/.venvs/flipper-rpc/bin/pip install flipperzero-protobuf pillow

Пример:
    fp = connect("/dev/cu.usbmodemflip_XXXX")
    press(fp, "OK")
    save_snapshot(fp, "menu.png")
    print(ascii_dump(fp.rpc_gui_snapshot_screen()))        # проверить, что разбор верный
    wait_for_frac(fp, (40, 92, 17, 28), 0.6, timeout=70)   # дождаться DONE

Перезапуск и сохранность данных — только через обычный pyserial, до RPC-сессии:
    restart_app("/dev/cu.usbmodemflip_XXXX", "/ext/apps/Tools/my_app.fap")
    fp = connect("/dev/cu.usbmodemflip_XXXX")
"""

from __future__ import annotations

import os
import time

from PIL import Image
from flipperzero_protobuf import FlipperProto


def connect(port: str, settle: float = 0.5) -> FlipperProto:
    """Подключиться к устройству. Порт указывать обязательно (иначе можно зависнуть)."""
    fp = FlipperProto(port)
    time.sleep(settle)
    return fp


def restart_app(port: str, fap: str = "/ext/apps/Tools/my_app.fap", settle: float = 2.5) -> None:
    """Перезапустить FAP через CLI `loader close` + `loader open`.

    RPC-методы для этого не годятся: `rpc_app_exit()` для приложения, запущенного
    через `ufbt launch`/`loader open`, отвечает `ERROR_APP_NOT_RUNNING`, а
    `rpc_app_start()` на уже работающем — `ERROR_APP_SYSTEM_LOCKED`.
    Порт должен быть свободен от RPC-сессии.
    """
    import serial

    ser = serial.Serial(port, 115200, timeout=0.3)
    try:
        time.sleep(0.8)
        ser.write(b"\x03")  # Ctrl+C: выйти из возможного хвоста вывода
        time.sleep(1.0)
        ser.reset_input_buffer()
        for command in ("loader close", "loader open %s" % fap):
            ser.write(command.encode() + b"\r\n")
            time.sleep(2.0)
            ser.reset_input_buffer()
    finally:
        ser.close()
    time.sleep(settle)


def ascii_dump(data: bytes, y0: int = 0, y1: int = 63) -> str:
    """Распечатать кадр текстом. Мелкий шрифт должен читаться — иначе раскладка неверная."""
    lines = []
    for y in range(y0, y1 + 1):
        page, bit = (y // 8) * 128, 1 << (y % 8)
        lines.append("".join("#" if data[page + x] & bit else "." for x in range(128)))
    return "\n".join(lines)


def framebuffer_to_image(data: bytes) -> Image.Image:
    """Преобразовать 1024-байтный кадр в монохромное изображение 128x64."""
    if len(data) != 128 * 64 // 8:
        raise ValueError(f"unexpected framebuffer size: {len(data)}")
    im = Image.new("1", (128, 64), 1)
    px = im.load()
    for y in range(64):
        page, bit = (y // 8) * 128, 1 << (y % 8)
        for x in range(128):
            if data[page + x] & bit:
                px[x, y] = 0
    return im


def save_snapshot(fp: FlipperProto, path: str, scale: int = 4) -> str:
    """Снять кадр и сохранить PNG (увеличенным, чтобы было видно глазами)."""
    im = framebuffer_to_image(fp.rpc_gui_snapshot_screen())
    if scale != 1:
        im = im.resize((128 * scale, 64 * scale), Image.NEAREST)
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    im.save(path)
    return path


def press(fp: FlipperProto, key: str = "OK", wait: float = 0.5) -> None:
    """Короткое нажатие. key: UP/DOWN/LEFT/RIGHT/OK/BACK."""
    fp.rpc_gui_send_input(f"SHORT {key}")
    if wait:
        time.sleep(wait)


def dark_frac(data: bytes, rect: tuple[int, int, int, int]) -> float:
    """Доля чёрных пикселей в прямоугольнике (x0, x1, y0, y1) включительно."""
    x0, x1, y0, y1 = rect
    dark = total = 0
    for y in range(y0, y1 + 1):
        page, bit = (y // 8) * 128, 1 << (y % 8)
        for x in range(x0, x1 + 1):
            total += 1
            if data[page + x] & bit:
                dark += 1
    return dark / total


def wait_for_frac(
    fp: FlipperProto,
    rect: tuple[int, int, int, int],
    threshold: float,
    timeout: float,
    greater: bool = True,
    poll: float = 0.1,
) -> float | None:
    """Ждать, пока доля тёмного в rect не станет > (или <) threshold.

    Возвращает время ожидания в секундах или None по таймауту.
    Порог подбирается по двум снятым кадрам: см. references/rpc-ui-testing.md.
    """
    start = time.monotonic()
    while time.monotonic() - start < timeout:
        frac = dark_frac(fp.rpc_gui_snapshot_screen(), rect)
        if (frac > threshold) == greater:
            return time.monotonic() - start
        time.sleep(poll)
    return None


def list_dir(fp: FlipperProto, path: str) -> list[str]:
    """Список имён в каталоге. Путь — реальный, без завершающего слэша."""
    return [item["name"] for item in fp.rpc_storage_list(path)]


def write_text(fp: FlipperProto, path: str, text: str) -> None:
    fp.rpc_write(path, text)


def read_text(fp: FlipperProto, path: str) -> str:
    return fp.rpc_read(path).decode("utf-8", "replace")


def backup_dir(fp: FlipperProto, remote_dir: str, local_dir: str) -> dict[str, bytes]:
    """Скопировать все файлы каталога на диск. Возвращает снимок содержимого."""
    os.makedirs(local_dir, exist_ok=True)
    snapshot: dict[str, bytes] = {}
    for name in list_dir(fp, remote_dir):
        data = fp.rpc_read("%s/%s" % (remote_dir, name))
        snapshot[name] = data
        with open(os.path.join(local_dir, name), "wb") as handle:
            handle.write(data)
    return snapshot


def restore_dir(
    fp: FlipperProto, remote_dir: str, local_dir: str, clear: bool = True
) -> list[str]:
    """Вернуть файлы из локальной копии и сверить побайтно. Возвращает список расхождений."""
    if clear:
        for name in list_dir(fp, remote_dir):
            fp.rpc_delete("%s/%s" % (remote_dir, name))
    mismatched = []
    for name in sorted(os.listdir(local_dir)):
        with open(os.path.join(local_dir, name), "rb") as handle:
            expected = handle.read()
        fp.rpc_write("%s/%s" % (remote_dir, name), expected)
        if fp.rpc_read("%s/%s" % (remote_dir, name)) != expected:
            mismatched.append(name)
    return mismatched


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 2:
        print("usage: flipper_rpc.py <port> [snapshot.png | --ascii]")
        raise SystemExit(2)
    device = connect(sys.argv[1])
    if len(sys.argv) > 2 and sys.argv[2] == "--ascii":
        print(ascii_dump(device.rpc_gui_snapshot_screen()))
    else:
        out = sys.argv[2] if len(sys.argv) > 2 else "snapshot.png"
        print(save_snapshot(device, out))
