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

#include "views/fn_test_view_detect.h"

#include "lib/fn/fn_worker.h"
#include "lib/fn/fn.h"
#include "lib/fn/fn_errors.h"


#define TAG "FNTest"

typedef enum {
    FNModeGetFNInfo,
    FNModeGetFNLifeInfo,
    FNModeGetLastDocInfo,
    FNModeGetRegInfo,
    FNModeGetSessionInfo,
    FNModeTestSell,
    FNModeCloseSession,
    FNModeFlashMGM,
    FNModeCloseFN,
    FNModeUnknown
} FNAppMode;

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
    FNError last_fn_error;

    FNAppMode mode;
    FNWorker* worker;

    FNInfo* fn_info;
    void* fn_tmp_data;

    FNDetectView* view_detect;
};

typedef enum {
    FNTestViewSubmenu,
    FNTestViewDialogEx,
    FNTestViewPopup,
    FNTestViewWidget,
    FNTestViewProgress,
    FNTestViewDetect
} FNAppView;
