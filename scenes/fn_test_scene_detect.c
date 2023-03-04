#include "../fn_test_app_i.h"
#include "applications_user/fn_test/lib/fn/fn_uart_i.h"

#define TAG_S "SCENE DETECT"

static void fn_test_scene_chip_detect_callback(void* context, FNCustomEventWorker event) {
    FNApp * app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void uart_cb(uint8_t* buf, size_t len, void* context)
{
    UARTApp* app = context;
    UNUSED(buf);
    UNUSED(len);
    UNUSED(context);
    if(app->state == UARTModeRxDone){
        FURI_LOG_D(TAG, "RX TIMEOUT, DATA LEN %d", len);
        for(int i = 0; i < (int)len; ++i) {
            FURI_LOG_D(TAG, " %04x", buf[i]);
        }
    }
}

void fn_test_scene_detect_on_enter(void* context) {
    FNApp * app = context;
    notification_message(app->notifications, &sequence_blink_start_yellow);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewDetect);

    UARTApp *uartApp = fn_uart_app_alloc();
    fn_uart_set_rx_callback(uartApp,uart_cb);
    uint8_t cmd[6];
    cmd[0] = 0x04;
    cmd[1] = 0x01;
    cmd[2] = 0x00;
    cmd[3] = 0x31;
    cmd[4] = 0xDE;
    cmd[5] = 0xDD;

    //furi_hal_uart_set_irq_cb(FuriHalUartIdLPUART1, uart_cb, app);
    fn_uart_trx(uartApp, cmd, 6, 50);
    furi_delay_ms(1000);
    fn_uart_app_free(uartApp);


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
