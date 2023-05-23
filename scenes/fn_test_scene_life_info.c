//
// Created by Игорь Данилов on 11.03.2023.
//
#include "../fn_test_app_i.h"
#include "applications_user/fn_test/lib/fn/fn_objects/life_info/fn_life_info.h"
#include "applications_user/fn_test/lib/fn/fn_objects/life_info/fn_life_info.h"
#include "../views/fn_test_view_progress.h"

#define FN_TEST_NAME "\e#\e!       Life Info        \e!\n"
#define FN_TEST_BLANK_INV "\e#\e!                                                      \e!\n"

static void fn_test_scene_life_info_callback(void* context, FNCustomEventWorker event, FNError fn_error) {
    UNUSED(fn_error);
    furi_check(context);
    furi_check(event);
    FNApp * app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void fn_test_scene_life_info_on_enter(void* context) {
    furi_check(context);
    FNApp * app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewProgress);
    fn_test_view_progress_set_mode(app->view_progress, FNProgressViewTypeWaitData);
    app->fn_tmp_data = fn_life_info_alloc();
    fn_worker_start_thread(app->worker);
    fn_worker_get_life_info_start(app->fn_tmp_data, app->worker, fn_test_scene_life_info_callback, app);
}

bool fn_test_scene_life_info_on_event(void* context, SceneManagerEvent event) {
    furi_check(context);
    FNApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FNAppSceneStart);
    } else if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == FNCustomEventWorkerDone){
            FuriString* tmp_string = furi_string_alloc();
            FNLifeInfo* life_info = app->fn_tmp_data;

            widget_add_text_box_element(
                app->widget, 0, 0, 128, 14, AlignCenter, AlignBottom, FN_TEST_BLANK_INV, false);
            widget_add_text_box_element(
                app->widget, 0, 2, 128, 14, AlignCenter, AlignBottom, FN_TEST_NAME, false);

            furi_string_printf(tmp_string, "\e#%s\n", "Validity period (32h)");
            furi_string_cat_printf(tmp_string, "End date: ");
            fn_life_info_get_end_date(life_info, tmp_string);
            furi_string_cat_printf(tmp_string, "\nReg report count: %d\n", fn_life_info_get_reg_report_ctn(life_info));
            furi_string_cat_printf(tmp_string, "Reg report remaining: %d\n", fn_life_info_get_reg_report_ctn_remaining(life_info));
            if(fn_get_max_ffd_enum(app->fn_info) >= FFD_1_1){
                furi_string_cat_printf(tmp_string, "\e#%s\n", "Remaining term (3Bh)");
                furi_string_cat_printf(tmp_string, "Days to end: %d\n", fn_life_info_get_days_to_end(life_info));
            }
            if(fn_get_max_ffd_enum(app->fn_info) >= FFD_1_2){
                furi_string_cat_printf(tmp_string, "\e#%s\n", "Free memory (3Dh)");
                furi_string_cat_printf(tmp_string, "Five year data resource:\n %lu\n", fn_life_info_get_five_year_data_resource(life_info));
                furi_string_cat_printf(tmp_string, "Thirty year data resource:\n %lu\n", fn_life_info_get_thirty_year_data_resource(life_info));
                furi_string_cat_printf(tmp_string, "Marking notifications \nresource: %lu\n", fn_life_info_get_marking_notifications_resource(life_info));
            }


            widget_add_text_scroll_element(app->widget, 0, 16, 128, 50, furi_string_get_cstr(tmp_string));

            furi_string_free(tmp_string);
            view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewWidget);
        } else if(event.event == FNCustomEventWorkerFNNotResponse)
        {
            scene_manager_next_scene(app->scene_manager, FNAppSceneTimeout);
        }
    }
    return success;
}
void fn_test_scene_life_info_on_exit(void* context) {
    furi_check(context);
    FNApp * app = context;
    widget_reset(app->widget);
    fn_worker_stop_thread(app->worker);
    fn_life_info_free(app->fn_tmp_data);
}
