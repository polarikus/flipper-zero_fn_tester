#pragma once
#include <gui/view.h>

typedef struct FNProgressView FNProgressView;
typedef void (*FNProgressViewCallback)(void* context);
typedef enum {
    FNProgressViewTypeWaitData,
    FNProgressViewTypeSendData,
    FNProgressViewTypeFlash
} FNTestProgressViewMode;

View* fn_test_view_progress_get_view(FNProgressView* app);
FNProgressView* fn_test_view_progress_alloc();
void fn_test_view_progress_free(FNProgressView* app);
void fn_test_view_progress_set_mode(FNProgressView* app, FNTestProgressViewMode mode);
