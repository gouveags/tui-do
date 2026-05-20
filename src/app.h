#ifndef TUI_DO_APP_H
#define TUI_DO_APP_H

#include "terminal.h"
#include "ui.h"

typedef struct App {
    AppState state;
    TerminalSize terminal;
    TerminalSize pending_terminal;
    int pending_resize_observations;
    int needs_redraw;
} App;

App app_create(TerminalSize terminal);
void app_handle_resize(App *app, TerminalSize terminal);
void app_observe_terminal_size(App *app, TerminalSize terminal);

#endif
