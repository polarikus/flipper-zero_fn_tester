#include "../fn_test_app_i.h"

static void fn_test_scene_chip_detect_callback(void* context, FNCustomEventWorker event) {
    FNApp * app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void fn_test_scene_detect_on_enter(void* context) {
    FNApp * app = context;
    notification_message(app->notifications, &sequence_blink_start_yellow);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewDetect);
    //spi_mem_worker_start_thread(app->worker);
    //spi_mem_worker_chip_detect_start(
      //  app->chip_info, &app->found_chips, app->worker, spi_mem_scene_chip_detect_callback, app);
}

bool fn_test_scene_detect_on_event(void* context, SceneManagerEvent event) {
    FNApp * app = context;
    UNUSED(app);
    UNUSED(fn_test_scene_chip_detect_callback);
    bool success = false;
    if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == SPIMemCustomEventWorkerChipIdentified) {
           // scene_manager_set_scene_state(app->scene_manager, SPIMemSceneSelectVendor, 0);
            //scene_manager_next_scene(app->scene_manager, SPIMemSceneSelectVendor);
        } else if(event.event == SPIMemCustomEventWorkerChipUnknown) {
            //scene_manager_next_scene(app->scene_manager, SPIMemSceneChipDetectFail);
        }
    }
    return success;
}

void fn_test_scene_detect_on_exit(void* context) {
    FNApp * app = context;
    //fn_test_worker_stop_thread(app->worker);
    notification_message(app->notifications, &sequence_blink_stop);
    //popup_reset(app->popup);
}
