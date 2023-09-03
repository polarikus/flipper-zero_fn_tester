#include "../fn_test_app_i.h"

#define FN_TEST_VERSION_APP "0.1.0-alpha01"
#define FN_TEST_DEVELOPER "Polarikus"
#define FN_TEST_GITHUB "https://github.com/polarikus/flipper-zero-fn_test"
#define FN_TEST_NAME "\e#\e!       FN Tester        \e!\n"
#define FN_TEST_BLANK_INV "\e#\e!                                                      \e!\n"

void fn_test_scene_about_on_enter(void* context) {
    FNApp* app = context;
    FuriString* tmp_string = furi_string_alloc();

    widget_add_text_box_element(
        app->widget, 0, 0, 128, 14, AlignCenter, AlignBottom, FN_TEST_BLANK_INV, false);
    widget_add_text_box_element(
        app->widget, 0, 2, 128, 14, AlignCenter, AlignBottom, FN_TEST_NAME, false);
    furi_string_printf(tmp_string, "\e#%s\n", "Information");
    furi_string_cat_printf(tmp_string, "Version: %s\n", FN_TEST_VERSION_APP);
    furi_string_cat_printf(tmp_string, "Developed by: %s\n", FN_TEST_DEVELOPER);
    furi_string_cat_printf(tmp_string, "Github: %s\n\n", FN_TEST_GITHUB);
    furi_string_cat_printf(tmp_string, "\e#%s\n", "Description");
    furi_string_cat_printf(
        tmp_string,
        "Tester for fiscal memory.\n"
        "Connect FN to flipper and \nread FN info or run commands\n");
    widget_add_text_scroll_element(app->widget, 0, 16, 128, 50, furi_string_get_cstr(tmp_string));

    furi_string_free(tmp_string);
    view_dispatcher_switch_to_view(app->view_dispatcher, FNTestViewWidget);
}

bool fn_test_scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void fn_test_scene_about_on_exit(void* context) {
    FNApp* app = context;
    widget_reset(app->widget);
}
