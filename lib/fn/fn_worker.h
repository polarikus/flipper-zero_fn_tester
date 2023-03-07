//
// Created by Игорь Данилов on 23.02.2023.
//
#pragma once

#include <furi.h>
#include "fn.h"

#define APP_DEBUG

typedef struct FNWorker FNWorker;

typedef struct {
    void (*const process)(FNWorker* worker);
} FNWorkerModeType;

typedef enum {
    FNCustomEventWorkerFNIdentified,
    FNCustomEventWorkerFNNotResponse,
    FNCustomEventWorkerFNError,
    FNCustomEventWorkerDone
} FNCustomEventWorker;

typedef enum {
    FNRxStatusOk,
    FNRxEmpty,
    FNRxStreamError,
    FNRxTimeout
} RxStatus;

typedef void (*FNWorkerCallback)(void* context, FNCustomEventWorker event);

void uart_thread_cb(uint8_t* buff, size_t len ,void * ctx);

/**
 * Выделить память под воркер (10240 стек задачи)
 * @return указатель на воркер
 */
FNWorker* fn_worker_alloc();
/**
 * Высвободить память воркера; Перед этим нужно завершить задачу
 * @param worker
 */
void fn_worker_free(FNWorker* worker);
/**
 * Запустить задачу воркера
 * @param worker
 */
void fn_worker_start_thread(FNWorker* worker);
/**
 * Остановить задачу воркера
 * @param worker
 */
void fn_worker_stop_thread(FNWorker* worker);
/**
 * Проверить остановлен ли воркер
 * @param worker
 * @return
 */
bool fn_worker_check_for_stop(FNWorker* worker);

RxStatus fn_worker_wait_uart_cb(FNWorker* worker, uint32_t timeout);

/**
 * Определить подключен ли ФН
 * @param fn_info Указатель на FNInfo с основной информацией о подключенном ФН
 * @param worker Указатель на FN воркер
 * @param callback Колбек, куда придёт статус по завершению задачи
 * @param context Контекст с приложением, которое вызвало функцию
 */
void fn_worker_fn_detect_start(
    FNInfo* fn_info,
    FNWorker* worker,
    FNWorkerCallback callback,
    void* context);


