#pragma once

#include <gui/scene_manager.h>

// Enum сцен + количество: генерируется из списка в my_app_scene_config.h
#define ADD_SCENE(prefix, name, id) MyAppScene##id,
typedef enum {
#include "my_app_scene_config.h"
    MyAppSceneNum,
} MyAppScene;
#undef ADD_SCENE

extern const SceneManagerHandlers my_app_scene_handlers;

// Объявления on_enter
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_enter(void* context);
#include "my_app_scene_config.h"
#undef ADD_SCENE

// Объявления on_event
#define ADD_SCENE(prefix, name, id) \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event);
#include "my_app_scene_config.h"
#undef ADD_SCENE

// Объявления on_exit
#define ADD_SCENE(prefix, name, id) void prefix##_scene_##name##_on_exit(void* context);
#include "my_app_scene_config.h"
#undef ADD_SCENE
