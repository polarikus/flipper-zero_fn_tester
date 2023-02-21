#pragma once

#include "fn_test_app.h"
#include "scenes/fn_test_scene.h"

#include <fn_test_icons.h>
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>

#define RECORD_STORAGE "storage"
#define BC_SCANNER_APP_PATH_FOLDER ANY_PATH("bcscanner")
#define BC_SCANNER_APP_EXTENSION ".bc"

typedef enum {
    BarCodeAppErrorNoFiles,
    BarCodeAppErrorCloseRpc,
} FNAppError;

struct FNApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    DialogEx* dialog_ex;
    Widget* widget;
    Submenu* submenu;
    Popup* popup;

    FNAppError error;
    FuriString* file_path;
};

typedef enum {
    FNTestViewSubmenu,
    FNTestViewDialogEx,
    FNTestViewPopup,
    FNTestViewWidget
} FNAppView;
