#!/usr/bin/env bash
# Статическая проверка правил слоёв (этап R0, задача T004).
#
# Источник правил: specs/001-fn-tester-ux/contracts/module-boundaries.md §2.
#   * lib/fn/core, lib/fn/proto  — без furi / gui/ / furi_hal;
#   * lib/fn                     — без fn_test_app / scene_ / view_;
#   * ui/                        — без fn_uart / fn_frame / fn_parse_ /
#                                  fn_wire / FN_CMD;
#   * ui/fn_format.c, ui/fn_text_model.h, ui/fn_strings.h
#                                — дополнительно без gui/ / view_ / scene_;
#   * ui/views, ui/scenes        — без fn_parse_ / fn_wire / FN_CMD.
#
# Отсутствующие каталоги/файлы пропускаются: до этапов R1–R2 их ещё нет.
# Код возврата: 0 — нарушений нет, 1 — есть.

set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
failed=0
checked=0

# scan <описание> <regex> <путь>...
scan() {
    local label="$1"
    local pattern="$2"
    shift 2

    local existing=()
    local path
    for path in "$@"; do
        [ -e "$path" ] && existing+=("$path")
    done

    if [ ${#existing[@]} -eq 0 ]; then
        printf '  - %s: путей нет, пропуск\n' "$label"
        return
    fi

    checked=$((checked + 1))

    local hits
    # Комментарии не считаем нарушением: в них допустимо упоминать запрещённые
    # имена (например, «файл не включает gui/ и furi»).
    hits="$(grep -rnE -- "$pattern" "${existing[@]}" 2>/dev/null \
        | grep -vE '^[^:]+:[0-9]+:[[:space:]]*(//|\*|/\*)' || true)"
    if [ -n "$hits" ]; then
        printf '  x %s: запрещено /%s/\n' "$label" "$pattern"
        printf '%s\n' "$hits" | sed 's/^/      /'
        failed=1
    else
        printf '  + %s\n' "$label"
    fi
}

printf 'Проверка слоёв (contracts/module-boundaries.md §2):\n'

scan "core/ без furi и gui" \
    '(^|[^a-zA-Z0-9_])furi|gui/|furi_hal' \
    "$ROOT/lib/fn/core"

scan "proto/ без furi и gui" \
    '(^|[^a-zA-Z0-9_])furi|gui/|furi_hal' \
    "$ROOT/lib/fn/proto"

scan "lib/fn без UI-зависимостей" \
    'fn_test_app|scene_|view_' \
    "$ROOT/lib/fn"

scan "ui/ без транспорта и кодов команд" \
    'fn_uart|fn_frame|fn_parse_|fn_wire|FN_CMD' \
    "$ROOT/ui"

scan "ui/fn_format.c без GUI" \
    'gui/|view_|scene_' \
    "$ROOT/ui/fn_format.c"

scan "ui/fn_text_model.h без GUI" \
    'gui/|view_|scene_' \
    "$ROOT/ui/fn_text_model.h"

scan "ui/fn_strings.h без GUI" \
    'gui/|view_|scene_' \
    "$ROOT/ui/fn_strings.h"

scan "ui/views и ui/scenes без протокола" \
    'fn_parse_|fn_wire|FN_CMD' \
    "$ROOT/ui/views" \
    "$ROOT/ui/scenes"

if [ "$failed" -ne 0 ]; then
    printf 'LAYERS: FAILED\n'
    exit 1
fi

printf 'LAYERS: PASSED (проверено правил: %d)\n' "$checked"
exit 0
