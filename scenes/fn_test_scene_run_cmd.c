//
// Created by Игорь Данилов on 21.03.2023.
//
#include "../fn_test_app_i.h"
#include "../lib/fn/fn_helpers.h"
#define S_TAG "SCENE_RUN_CMD"

static void
    fn_test_scene_run_cmd_callback(void* context, FNCustomEventWorker event, FNError fn_error) {
    UNUSED(fn_error);
    furi_check(context);
    furi_check(event);
    FNApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void fn_test_scene_run_cmd_on_enter(void* context) {
    furi_check(context);
    FNApp* app = context;
    fn_worker_start_thread(app->worker);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewProgress);
    if(app->mode == FNModeFlashMGM) {
        fn_test_view_progress_set_mode(app->view_progress, FNProgressViewTypeFlash);
        fn_worker_flash_MGM_start(app->worker, fn_test_scene_run_cmd_callback, app);
    }
}

bool fn_test_scene_run_cmd_on_event(void* context, SceneManagerEvent event) {
    furi_check(context);
    FNApp* app = context;
    UNUSED(event);
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FNAppSceneStart);
    } else if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        widget_add_string_element(
            app->widget, 64, 9, AlignCenter, AlignBottom, FontPrimary, "Result");

        if(event.event == FNCustomEventWorkerDone) {
            widget_add_string_element(
                app->widget, 64, 21, AlignCenter, AlignBottom, FontSecondary, "Ok");
            view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewWidget);
        } else if(event.event == FNCustomEventWorkerFNNotResponse) {
            scene_manager_next_scene(app->scene_manager, FNAppSceneTimeout);
        } else if(event.event == FNCustomEventWorkerFNError) {
            widget_add_string_element(
                app->widget, 64, 21, AlignCenter, AlignBottom, FontSecondary, "FN Error!");
            view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewWidget);
        }
    }
    return success;
}

void fn_test_scene_run_cmd_on_exit(void* context) {
    furi_check(context);
    FNApp* app = context;
    widget_reset(app->widget);
    fn_worker_stop_thread(app->worker);
}
