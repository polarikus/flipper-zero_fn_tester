#pragma once

#include "my_app.h"
#include "scenes/my_app_scene.h"

struct MyApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    Submenu* submenu;
    Widget* widget;
};
