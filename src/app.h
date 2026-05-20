#ifndef TUI_DO_APP_H
#define TUI_DO_APP_H

#include "terminal.h"
#include "ui.h"
#include "storage.h"

typedef struct App {
    AppState state;
    TerminalSize terminal;
    TerminalSize pending_terminal;
    int pending_resize_observations;
    int needs_redraw;
    int should_quit;
    char storage_root[TODO_PATH_MAX];
    long long now;
    int next_id;
} App;

App app_create(TerminalSize terminal);
App app_create_with_storage(TerminalSize terminal, const char *storage_root);
void app_handle_resize(App *app, TerminalSize terminal);
void app_observe_terminal_size(App *app, TerminalSize terminal);
void app_handle_key(App *app, int key);

#endif
