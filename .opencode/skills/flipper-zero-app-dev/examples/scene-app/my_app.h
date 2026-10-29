#pragma once

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>

typedef struct MyApp MyApp;

typedef enum {
    MyAppViewSubmenu,
    MyAppViewWidget,
} MyAppView;
