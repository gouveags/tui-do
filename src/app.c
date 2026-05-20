#include "app.h"

App app_create(TerminalSize terminal) {
    return (App) {
        .state = {
            .selected_menu_index = 0,
        },
        .terminal = terminal,
        .pending_terminal = terminal,
        .pending_resize_observations = 0,
        .needs_redraw = 1,
        .should_quit = 0,
    };
}

void app_handle_resize(App *app, TerminalSize terminal) {
    if (app->terminal.width != terminal.width || app->terminal.height != terminal.height) {
        app->terminal = terminal;
        app->pending_terminal = terminal;
        app->pending_resize_observations = 0;
        app->needs_redraw = 1;
    }
}

void app_observe_terminal_size(App *app, TerminalSize terminal) {
    if (terminal.width <= 0 || terminal.height <= 0) {
        return;
    }

    if (app->terminal.width == terminal.width && app->terminal.height == terminal.height) {
        app->pending_terminal = terminal;
        app->pending_resize_observations = 0;
        return;
    }

    if (
        app->pending_terminal.width == terminal.width &&
        app->pending_terminal.height == terminal.height
    ) {
        app->pending_resize_observations++;
    } else {
        app->pending_terminal = terminal;
        app->pending_resize_observations = 1;
    }

    if (app->pending_resize_observations >= 2) {
        app_handle_resize(app, terminal);
    }
}

void app_handle_key(App *app, int key) {
    int previous_selection = app->state.selected_menu_index;

    if (key == TERMINAL_KEY_CTRL_C || key == 'q') {
        app->should_quit = 1;
        return;
    }

    if (key == TERMINAL_KEY_UP) {
        app->state.selected_menu_index = (app->state.selected_menu_index + 2) % 3;
    } else if (key == TERMINAL_KEY_DOWN) {
        app->state.selected_menu_index = (app->state.selected_menu_index + 1) % 3;
    } else if (key >= '1' && key <= '3') {
        app->state.selected_menu_index = key - '1';
    } else if (key == TERMINAL_KEY_ENTER) {
        if (app->state.selected_menu_index == 2) {
            app->should_quit = 1;
        }
    }

    if (previous_selection != app->state.selected_menu_index) {
        app->needs_redraw = 1;
    }
}
