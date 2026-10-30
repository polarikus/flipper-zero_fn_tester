#include "fn_view_text.h"

#include "../fn_draw.h"
#include "../fn_ui.h"

#include <gui/canvas.h>
#include <gui/elements.h>
#include <input/input.h>

#include <furi.h>
#include <stdio.h>
#include <string.h>

#define FN_TEXT_VIEW_TOP      1
#define FN_TEXT_VIEW_H        (64 - FN_TEXT_VIEW_TOP)
#define FN_TEXT_MAX_WRAP_ROWS 4

typedef struct {
    FnTextModel model;
    const Icon* image;
    uint16_t scroll_y;
    uint16_t content_height;
} FnTextViewModel;

struct FnTextView {
    View* view;
};

static uint16_t
    fn_view_text_layout(Canvas* canvas, const FnTextModel* model, bool draw, uint16_t scroll) {
    int32_t y = FN_TEXT_VIEW_TOP - (int32_t)scroll;
    uint16_t total = FN_TEXT_VIEW_TOP;

    for(uint8_t s = 0; s < model->section_count; ++s) {
        const FnTextSection* section = &model->sections[s];

        if(section->title) {
            fn_draw_primary(canvas);
            const uint16_t h = fn_draw_measure(
                canvas, section->title, FN_UI_TEXT_W - FN_UI_MARGIN_X, FN_UI_LINE_H, 2);
            if(draw) {
                fn_draw_wrapped(
                    canvas,
                    FN_UI_MARGIN_X,
                    y,
                    section->title,
                    FN_UI_TEXT_W - FN_UI_MARGIN_X,
                    FN_UI_LINE_H,
                    2);
            }
            y += h;
            total += h;
        }

        fn_draw_small(canvas);
        for(uint8_t l = 0; l < section->line_count; ++l) {
            const FnTextLine* line = &section->lines[l];
            const uint16_t h = fn_draw_measure(
                canvas,
                line->text,
                FN_UI_TEXT_W - FN_UI_MARGIN_X,
                FN_UI_SMALL_H,
                FN_TEXT_MAX_WRAP_ROWS);
            if(draw) {
                fn_draw_wrapped(
                    canvas,
                    FN_UI_MARGIN_X,
                    y,
                    line->text,
                    FN_UI_TEXT_W - FN_UI_MARGIN_X,
                    FN_UI_SMALL_H,
                    FN_TEXT_MAX_WRAP_ROWS);
            }
            y += h;
            total += h;
        }

        y += FN_UI_SECTION_GAP;
        total += FN_UI_SECTION_GAP;
    }

    return total;
}

static void fn_view_text_draw_callback(Canvas* canvas, void* context) {
    FnTextViewModel* model = context;
    furi_assert(model);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    if(model->image) {
        canvas_draw_icon(canvas, 0, 0, model->image);
        return;
    }

    const uint16_t total = fn_view_text_layout(canvas, &model->model, false, 0);
    model->content_height = total;

    uint16_t scroll = model->scroll_y;
    const uint16_t max_scroll = (total > FN_TEXT_VIEW_H) ? (total - FN_TEXT_VIEW_H) : 0;
    if(scroll > max_scroll) {
        scroll = max_scroll;
        model->scroll_y = scroll;
    }

    fn_view_text_layout(canvas, &model->model, true, scroll);

    if(total > FN_TEXT_VIEW_H) {
        elements_scrollbar(canvas, scroll, total);
    }
}

static bool fn_view_text_input_callback(InputEvent* event, void* context) {
    FnTextView* text_view = context;
    furi_assert(text_view);

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;

    bool consumed = false;
    with_view_model(
        text_view->view,
        FnTextViewModel * model,
        {
            const uint16_t max_scroll = (model->content_height > FN_TEXT_VIEW_H) ?
                                            (model->content_height - FN_TEXT_VIEW_H) :
                                            0;
            int32_t scroll = model->scroll_y;
            switch(event->key) {
            case InputKeyUp:
                scroll -= FN_UI_LINE_H;
                consumed = true;
                break;
            case InputKeyDown:
                scroll += FN_UI_LINE_H;
                consumed = true;
                break;
            case InputKeyLeft:
                scroll -= (FN_TEXT_VIEW_H - FN_UI_LINE_H);
                consumed = true;
                break;
            case InputKeyRight:
                scroll += (FN_TEXT_VIEW_H - FN_UI_LINE_H);
                consumed = true;
                break;
            default:
                break;
            }
            if(scroll < 0) scroll = 0;
            if(scroll > (int32_t)max_scroll) scroll = max_scroll;
            model->scroll_y = (uint16_t)scroll;
        },
        consumed);

    return consumed;
}

FnTextView* fn_view_text_alloc(void) {
    FnTextView* text_view = malloc(sizeof(FnTextView));
    text_view->view = view_alloc();
    view_set_context(text_view->view, text_view);
    view_allocate_model(text_view->view, ViewModelTypeLocking, sizeof(FnTextViewModel));
    view_set_draw_callback(text_view->view, fn_view_text_draw_callback);
    view_set_input_callback(text_view->view, fn_view_text_input_callback);

    with_view_model(
        text_view->view,
        FnTextViewModel * model,
        {
            memset(&model->model, 0, sizeof(model->model));
            model->image = NULL;
            model->scroll_y = 0;
            model->content_height = 0;
        },
        false);
    return text_view;
}

void fn_view_text_free(FnTextView* text_view) {
    if(!text_view) return;
    view_free(text_view->view);
    free(text_view);
}

View* fn_view_text_get_view(FnTextView* text_view) {
    return text_view ? text_view->view : NULL;
}

void fn_view_text_clear(FnTextView* text_view) {
    if(!text_view) return;
    with_view_model(
        text_view->view,
        FnTextViewModel * model,
        {
            memset(&model->model, 0, sizeof(model->model));
            model->image = NULL;
            model->scroll_y = 0;
            model->content_height = 0;
        },
        true);
}

void fn_view_text_add_section(FnTextView* text_view, const char* title) {
    if(!text_view) return;
    with_view_model(
        text_view->view,
        FnTextViewModel * model,
        {
            if(model->model.section_count < FN_TEXT_MAX_SECTIONS) {
                FnTextSection* section = &model->model.sections[model->model.section_count];
                memset(section, 0, sizeof(FnTextSection));
                section->title = title;
                model->model.section_count++;
            }
        },
        true);
}

void fn_view_text_add_line(FnTextView* text_view, FnTextStyle style, const char* fmt, ...) {
    if(!text_view || !fmt) return;

    char buffer[FN_TEXT_LINE_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    with_view_model(
        text_view->view,
        FnTextViewModel * model,
        {
            if(model->model.section_count == 0) {
                FnTextSection* section = &model->model.sections[0];
                memset(section, 0, sizeof(FnTextSection));
                section->title = NULL;
                model->model.section_count = 1;
            }
            FnTextSection* section = &model->model.sections[model->model.section_count - 1];
            if(section->line_count < FN_TEXT_MAX_LINES) {
                FnTextLine* line = &section->lines[section->line_count];
                memset(line, 0, sizeof(FnTextLine));
                strncpy(line->text, buffer, FN_TEXT_LINE_LEN - 1);
                line->emphasis = style;
                section->line_count++;
            }
        },
        true);
}

void fn_view_text_commit(FnTextView* text_view) {
    if(!text_view) return;
    with_view_model(
        text_view->view,
        FnTextViewModel * model,
        {
            model->scroll_y = 0;
            model->content_height = 0;
        },
        true);
}

void fn_view_text_set_model(FnTextView* text_view, const FnTextModel* model) {
    if(!text_view || !model) return;
    with_view_model(
        text_view->view,
        FnTextViewModel * vm,
        {
            memcpy(&vm->model, model, sizeof(FnTextModel));
            vm->image = NULL;
            vm->scroll_y = 0;
            vm->content_height = 0;
        },
        true);
}

void fn_view_text_set_image(FnTextView* text_view, const Icon* icon) {
    if(!text_view) return;
    with_view_model(
        text_view->view,
        FnTextViewModel * model,
        {
            memset(&model->model, 0, sizeof(model->model));
            model->image = icon;
            model->scroll_y = 0;
            model->content_height = 0;
        },
        true);
}
