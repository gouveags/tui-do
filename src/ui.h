#ifndef TUI_DO_UI_H
#define TUI_DO_UI_H

#include "../vendor/clay/clay.h"
#include "terminal.h"

typedef struct AppState {
    int selected_menu_index;
} AppState;

Clay_RenderCommandArray ui_render_main_menu(AppState *state, TerminalSize terminal);

#endif
