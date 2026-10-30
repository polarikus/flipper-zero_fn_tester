#include "../../fn_test_app_i.h"
#include "fn_scene.h"

/** Режим экрана ожидания по типу операции. */
static FnWaitMode fn_scene_wait_mode(FnOpKind kind) {
    switch(kind) {
    case FnOpFlashMgm:
        return FnWaitModeFlash;
    case FnOpReadInfo:
    case FnOpReadLife:
        return FnWaitModeRead;
    case FnOpDetect:
    default:
        return FnWaitModeDetect;
    }
}

/** Тексты ожидания: заголовок и подсказка подзаголовка. */
static void fn_scene_wait_texts(FnWaitMode mode, const char** title, const char** subtitle) {
    switch(mode) {
    case FnWaitModeFlash:
        *title = FN_STR_WAIT_FLASH;
        *subtitle = FN_STR_WAIT_FLASH_HINT;
        break;
    case FnWaitModeRead:
        *title = FN_STR_WAIT_READ;
        *subtitle = FN_STR_WAIT_READ_HINT;
        break;
    case FnWaitModeDetect:
    default:
        *title = FN_STR_WAIT_DETECT;
        *subtitle = FN_STR_WAIT_DETECT_HINT;
        break;
    }
}

void fn_scene_wait_on_enter(void* context) {
    furi_assert(context);
    FNApp* app = context;

    const FnOpKind kind = app->pending_op;
    const FnWaitMode mode = fn_scene_wait_mode(kind);
    /* Ожидание очистки МГМ не прерывается (FR-015, FR-023). */
    const bool cancelable = fn_op_is_cancellable(kind);

    /* Данные прошлой операции фазы жизни не должны попасть на новый экран. */
    if(kind == FnOpReadLife) fn_life_info_reset(app->fn_life_info);

    const char* title = NULL;
    const char* subtitle = NULL;
    fn_scene_wait_texts(mode, &title, &subtitle);
    fn_view_wait_set(app->view_wait, mode, title, subtitle, cancelable);

    view_dispatcher_switch_to_view(app->view_dispatcher, FNAppViewWait);

    memset(&app->op_result, 0, sizeof(app->op_result));
    app->result_reason = FnResultReasonOperation;

    if(!fn_worker_start(app->worker, kind, &app->task, fn_test_app_handle_worker_result, app)) {
        FURI_LOG_E(TAG, "не удалось запустить операцию %d: воркер занят", (int)kind);
        /* Результат «занято» доставляется тем же событием, что и обычный,
         * чтобы сцена показала понятный итог, а не зависла на ожидании. */
        app->op_result.id = app->op_id;
        app->op_result.kind = kind;
        app->op_result.status = FnOpStatusBusy;
        view_dispatcher_send_custom_event(app->view_dispatcher, FNAppCustomEventWorkerResult);
        return;
    }

    /* Идентификатор берём у воркера: результат с другим id — «поздний». */
    app->op_id = fn_worker_get_op_id(app->worker);
}

/** Preflight «МГМ/боевой» после Detect: подтверждение только для МГМ (FR-024). */
static void fn_scene_wait_handle_flash_preflight(FNApp* app) {
    app->flash_preflight = false;

    const FnInfo* info = app->fn_info;
    const bool is_mgm = info->has_fw && info->fw.is_mgm;

    if(!is_mgm) {
        FURI_LOG_W(TAG, "очистка МГМ недоступна: ФН не определён как МГМ");
        app->result_reason = FnResultReasonNotMgm;
        scene_manager_next_scene(app->scene_manager, FNAppSceneResult);
        return;
    }

    scene_manager_next_scene(app->scene_manager, FNAppSceneConfirm);
}

/** Переход по результату операции: успех — данные (US1/US2), иначе — результат. */
static void fn_scene_wait_handle_result(FNApp* app) {
    if(app->op_result.status == FnOpStatusOk) {
        switch(app->op_result.kind) {
        case FnOpReadInfo:
            scene_manager_next_scene(app->scene_manager, FNAppSceneInfo);
            return;
        case FnOpReadLife:
            scene_manager_next_scene(app->scene_manager, FNAppSceneLife);
            return;
        case FnOpDetect:
            if(app->flash_preflight) {
                fn_scene_wait_handle_flash_preflight(app);
                return;
            }
            break;
        default:
            break;
        }
    }

    app->flash_preflight = false;
    app->result_reason = FnResultReasonOperation;
    scene_manager_next_scene(app->scene_manager, FNAppSceneResult);
}

bool fn_scene_wait_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    FNApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom && event.event == FNAppCustomEventWorkerResult) {
        if(app->op_result.id != app->op_id) {
            FURI_LOG_W(TAG, "поздний результат операции %u", (unsigned)app->op_result.id);
            return true;
        }
        fn_scene_wait_handle_result(app);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack && !fn_op_is_cancellable(app->pending_op)) {
        /* FR-015: ожидание очистки МГМ не прерывается — ждём ответ или 30 с. */
        consumed = true;
    }

    return consumed;
}

void fn_scene_wait_on_exit(void* context) {
    furi_assert(context);
    FNApp* app = context;

    /* Очистку МГМ отменять запрещено (contracts/worker-events.md §3). */
    if(fn_op_is_cancellable(app->pending_op) && fn_worker_is_busy(app->worker)) {
        fn_worker_cancel(app->worker);
    }
}
