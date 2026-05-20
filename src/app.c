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
