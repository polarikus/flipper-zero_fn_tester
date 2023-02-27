#pragma once
#include <gui/view.h>

typedef struct FNDetectView FNDetectView;
typedef void (*FNDetectViewCallback)(void* context);

View* fn_test_view_detect_get_view(FNDetectView* app);
FNDetectView* fn_test_view_detect_alloc();
void fn_test_view_detect_free(FNDetectView* app);
