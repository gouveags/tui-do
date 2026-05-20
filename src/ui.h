#ifndef TUI_DO_UI_H
#define TUI_DO_UI_H

#include "../vendor/clay/clay.h"
#include "storage.h"
#include "terminal.h"

typedef enum AppView {
    APP_VIEW_MAIN_MENU,
    APP_VIEW_CAPTURE,
    APP_VIEW_INBOX,
    APP_VIEW_DETAIL,
} AppView;

typedef struct AppState {
    AppView view;
    int selected_menu_index;
    int selected_inbox_index;
    TodoIndex inbox;
    char capture_title[128];
    int capture_cursor;
    char message[128];
} AppState;

Clay_RenderCommandArray ui_render_app(AppState *state, TerminalSize terminal);
Clay_RenderCommandArray ui_render_main_menu(AppState *state, TerminalSize terminal);

#endif
