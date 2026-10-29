#include "my_app_scene.h"

// Таблицы обработчиков собираются из my_app_scene_config.h тем же макросом,
// что и enum: список сцен описан в одном месте.

static const AppSceneOnEnterCallback my_app_scene_on_enter_handlers[] = {
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
#include "my_app_scene_config.h"
#undef ADD_SCENE
};

static const AppSceneOnEventCallback my_app_scene_on_event_handlers[] = {
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
#include "my_app_scene_config.h"
#undef ADD_SCENE
};

static const AppSceneOnExitCallback my_app_scene_on_exit_handlers[] = {
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
#include "my_app_scene_config.h"
#undef ADD_SCENE
};

const SceneManagerHandlers my_app_scene_handlers = {
    .on_enter_handlers = my_app_scene_on_enter_handlers,
    .on_event_handlers = my_app_scene_on_event_handlers,
    .on_exit_handlers = my_app_scene_on_exit_handlers,
    .scene_num = MyAppSceneNum,
};
