#include "../../fn_test_app_i.h"
#include "fn_scene.h"

void fn_scene_about_on_enter(void* context) {
    furi_assert(context);
    FNApp* app = context;

    fn_view_text_clear(app->view_text);
    fn_view_text_add_section(app->view_text, FN_STR_ABOUT_TITLE);
    fn_view_text_add_line(app->view_text, FnTextStyleNormal, "%s", FN_STR_ABOUT_APP);
    fn_view_text_add_line(app->view_text, FnTextStyleNormal, "%s", FN_STR_ABOUT_AUTHOR);
    fn_view_text_add_line(app->view_text, FnTextStyleNormal, "%s", FN_STR_ABOUT_URL);
    fn_view_text_add_section(app->view_text, NULL);
    fn_view_text_add_line(app->view_text, FnTextStyleNormal, "%s", FN_STR_ABOUT_DESCRIPTION);
    fn_view_text_commit(app->view_text);

    view_dispatcher_switch_to_view(app->view_dispatcher, FNAppViewText);
}

bool fn_scene_about_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    UNUSED(context);
    UNUSED(event);
    return false;
}

void fn_scene_about_on_exit(void* context) {
    furi_assert(context);
    UNUSED(context);
}
