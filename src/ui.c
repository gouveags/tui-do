#include "ui.h"

#include <stdbool.h>
#include <string.h>

static const Clay_Color COLOR_BACKGROUND = {18, 22, 28, 255};
static const Clay_Color COLOR_SURFACE = {28, 35, 44, 255};
static const Clay_Color COLOR_SURFACE_ACTIVE = {33, 55, 55, 255};
static const Clay_Color COLOR_TEXT = {234, 239, 237, 255};
static const Clay_Color COLOR_MUTED = {143, 153, 156, 255};
static const Clay_Color COLOR_ACCENT = {70, 221, 181, 255};
static const Clay_Color COLOR_SELECTED = {219, 255, 242, 255};

static Clay_String clay_string(const char *text) {
    return (Clay_String) {
        .length = (int32_t)strlen(text),
        .chars = text,
    };
}

static void draw_text(const char *text, Clay_Color color) {
    CLAY_TEXT(
        clay_string(text),
        CLAY_TEXT_CONFIG({
            .textColor = color,
            .fontSize = 1,
        })
    );
}

static void draw_top_bar(void) {
    CLAY(CLAY_ID("TopBar"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIXED(3),
            },
            .padding = {
                .left = 3,
                .right = 3,
                .top = 1,
            },
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
        .backgroundColor = COLOR_SURFACE,
    }) {
        draw_text("tui-do", COLOR_ACCENT);
        CLAY(CLAY_ID("TopBarSpacer"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIXED(1),
                },
            },
        }) {}
        draw_text("main menu", COLOR_MUTED);
    }
}

static void draw_hero(bool compact) {
    CLAY(CLAY_ID("Hero"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIT(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = compact ? 0 : 1,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
            },
        },
    }) {
        if (compact) {
            draw_text("tui-do", COLOR_SELECTED);
        } else {
            draw_text("tui-do", COLOR_SELECTED);
            draw_text("Small plans, clearly kept.", COLOR_MUTED);
        }
    }
}

static void draw_menu_item(int index, const char *number, const char *title, const char *description, bool selected, bool compact) {
    CLAY(CLAY_IDI("MenuItem", index), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIXED(compact ? 3 : 4),
            },
            .padding = {
                .left = 2,
                .right = 2,
                .top = compact ? 0 : 1,
            },
            .childGap = 2,
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
        .backgroundColor = selected ? COLOR_SURFACE_ACTIVE : COLOR_SURFACE,
    }) {
        draw_text(selected ? ">" : " ", selected ? COLOR_ACCENT : COLOR_MUTED);
        draw_text(number, selected ? COLOR_ACCENT : COLOR_MUTED);
        CLAY(CLAY_IDI("MenuCopy", index), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIT(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        }) {
            draw_text(title, selected ? COLOR_SELECTED : COLOR_TEXT);
            draw_text(description, COLOR_MUTED);
        }
    }
}

static void draw_menu(AppState *state, bool compact) {
    CLAY(CLAY_ID("MenuPanel"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_PERCENT(compact ? 0.92f : 0.48f),
                .height = CLAY_SIZING_FIT(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = {
                .left = compact ? 0 : 1,
                .right = compact ? 0 : 1,
                .top = compact ? 0 : 1,
                .bottom = compact ? 0 : 1,
            },
            .childGap = 1,
        },
        .backgroundColor = {22, 28, 36, 255},
    }) {
        draw_menu_item(0, "01", "Create a new to-do", "Start with an empty list", state->selected_menu_index == 0, compact);
        draw_menu_item(1, "02", "Load a to-do", "Continue an existing list", state->selected_menu_index == 1, compact);
        draw_menu_item(2, "03", "Quit", "Return to the terminal", state->selected_menu_index == 2, compact);
    }
}

static void draw_footer(void) {
    CLAY(CLAY_ID("Footer"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIXED(2),
            },
            .padding = {
                .left = 3,
                .right = 3,
                .top = 0,
            },
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .childGap = 3,
        },
        .backgroundColor = COLOR_SURFACE,
    }) {
        draw_text("up/down", COLOR_ACCENT);
        draw_text("move", COLOR_MUTED);
        draw_text("enter", COLOR_ACCENT);
        draw_text("select", COLOR_MUTED);
        draw_text("1-3", COLOR_ACCENT);
        draw_text("quick select", COLOR_MUTED);
        draw_text("q", COLOR_ACCENT);
        draw_text("quit", COLOR_MUTED);
    }
}

Clay_RenderCommandArray ui_render_main_menu(AppState *state, TerminalSize terminal) {
    bool compact = terminal.width < 96 || terminal.height <= 28;

    Clay_SetLayoutDimensions((Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("Root"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = {
                .left = 2,
                .right = 2,
                .top = 1,
                .bottom = 1,
            },
            .childGap = compact ? 1 : 2,
        },
        .backgroundColor = COLOR_BACKGROUND,
    }) {
        draw_top_bar();
        CLAY(CLAY_ID("Body"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childGap = compact ? 1 : 4,
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                    .y = CLAY_ALIGN_Y_CENTER,
                },
            },
        }) {
            draw_hero(compact);
            draw_menu(state, compact);
        }
        draw_footer();
    }

    return Clay_EndLayout(0.0f);
}
