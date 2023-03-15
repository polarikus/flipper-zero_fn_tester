//
// Created by Игорь Данилов on 11.03.2023.
//
#include "../fn_test_app_i.h"
#include "fn_helpers.h"
#include "applications_user/fn_test/lib/fn/data_types/fn_data_types.h"

#define FN_TEST_NAME "\e#\e!       FN Info        \e!\n"
#define FN_TEST_BLANK_INV "\e#\e!                                                      \e!\n"

void fn_test_scene_fn_info_on_enter(void* context) {
    FNApp * app = context;
    FuriString* tmp_string = furi_string_alloc();

    widget_add_text_box_element(
        app->widget, 0, 0, 128, 14, AlignCenter, AlignBottom, FN_TEST_BLANK_INV, false);
    widget_add_text_box_element(
        app->widget, 0, 2, 128, 14, AlignCenter, AlignBottom, FN_TEST_NAME, false);

    furi_string_printf(tmp_string, "\e#%s\n", "FN Status (30h)");
    furi_string_cat_printf(tmp_string, "SN: %s\n", fn_get_sn(app->fn_info));
    furi_string_cat_printf(tmp_string, "FN Stage: %s\n",
                           fn_get_fn_state(app->fn_info));
    furi_string_cat_printf(tmp_string, "Session open: %s\n",
                           bool_to_hum(fn_is_session_open(app->fn_info)));
    if(fn_get_fn_state_enum(app->fn_info) > FNStage1){
        furi_string_cat_printf(tmp_string, "Last fd datetime:\n");
        fn_get_last_document_datetime(app->fn_info, tmp_string);
        furi_string_cat_printf(tmp_string, "\nLast fd num: %lu\n", fn_get_last_document_number(app->fn_info));
    }
    if(fn_is_warn_flags_not_null(app->fn_info)){
        furi_string_cat_printf(tmp_string, "Warning flags:");
        fn_get_warn_flags_str(app->fn_info, tmp_string);
    }
    furi_string_cat_printf(tmp_string, "\e#%s\n", "FN FFD (3Ah)");
    furi_string_cat_printf(tmp_string, "FFD version: %s\n", fn_get_ffd_version_string(app->fn_info));
    furi_string_cat_printf(tmp_string, "Max FFD version: %s\n", fn_get_max_ffd_version_string(app->fn_info));
    furi_string_cat_printf(tmp_string, "\e#%s\n", "FN version (33h)");
    furi_string_cat_printf(tmp_string, "FW version: %s\n", fn_get_fw_version(app->fn_info));
    furi_string_cat_printf(tmp_string, "Is MGM: %s\n", bool_to_hum(fn_is_mgm(app->fn_info)));

    widget_add_text_scroll_element(app->widget, 0, 16, 128, 50, furi_string_get_cstr(tmp_string));

    furi_string_free(tmp_string);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewWidget);
}

bool fn_test_scene_fn_info_on_event(void* context, SceneManagerEvent event) {
    FNApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FNAppSceneStart);
    } else if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, FNAppSceneStart);
        }
    }
    return success;
}
void fn_test_scene_fn_info_on_exit(void* context) {
    FNApp * app = context;
    widget_reset(app->widget);
}