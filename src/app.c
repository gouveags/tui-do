#include "app.h"

#include <stdio.h>
#include <string.h>

App app_create(TerminalSize terminal) {
    return app_create_with_storage(terminal, "data");
}

App app_create_with_storage(TerminalSize terminal, const char *storage_root) {
    App app = {
        .state = {
            .view = APP_VIEW_MAIN_MENU,
            .selected_menu_index = 0,
            .capture_cursor = 0,
        },
        .terminal = terminal,
        .pending_terminal = terminal,
        .pending_resize_observations = 0,
        .needs_redraw = 1,
        .should_quit = 0,
        .now = 0,
        .next_id = 1,
    };

    snprintf(app.storage_root, sizeof(app.storage_root), "%s", storage_root);
    return app;
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

static void capture_insert_char(App *app, char value) {
    size_t length = strlen(app->state.capture_title);
    int cursor = app->state.capture_cursor;

    if (length + 1 >= sizeof(app->state.capture_title)) {
        return;
    }
    if (cursor < 0 || cursor > (int)length) {
        cursor = (int)length;
    }

    memmove(
        app->state.capture_title + cursor + 1,
        app->state.capture_title + cursor,
        length - (size_t)cursor + 1
    );
    app->state.capture_title[cursor] = value;
    app->state.capture_cursor = cursor + 1;
    app->needs_redraw = 1;
}

static void capture_backspace(App *app) {
    size_t length = strlen(app->state.capture_title);
    int cursor = app->state.capture_cursor;

    if (cursor <= 0 || length == 0) {
        return;
    }
    if (cursor > (int)length) {
        cursor = (int)length;
    }

    memmove(
        app->state.capture_title + cursor - 1,
        app->state.capture_title + cursor,
        length - (size_t)cursor + 1
    );
    app->state.capture_cursor = cursor - 1;
    app->needs_redraw = 1;
}

void app_handle_key(App *app, int key) {
    int previous_selection = app->state.selected_menu_index;

    if (key == TERMINAL_KEY_CTRL_C || key == 'q') {
        app->should_quit = 1;
        return;
    }

    if (app->state.view == APP_VIEW_CAPTURE) {
        if (key == TERMINAL_KEY_BACKSPACE) {
            capture_backspace(app);
            return;
        }
        if (key == TERMINAL_KEY_LEFT) {
            if (app->state.capture_cursor > 0) {
                app->state.capture_cursor--;
                app->needs_redraw = 1;
            }
            return;
        }
        if (key == TERMINAL_KEY_RIGHT) {
            if (app->state.capture_cursor < (int)strlen(app->state.capture_title)) {
                app->state.capture_cursor++;
                app->needs_redraw = 1;
            }
            return;
        }
        if (key == TERMINAL_KEY_ENTER) {
            if (app->state.capture_title[0] == '\0') {
                return;
            }

            Todo todo = {
                .created_at = app->now,
                .updated_at = app->now,
                .item_count = 0,
            };
            snprintf(todo.id, sizeof(todo.id), "entry-%d", app->next_id++);
            snprintf(todo.title, sizeof(todo.title), "%s", app->state.capture_title);
            if (storage_save_todo(app->storage_root, &todo) == 0) {
                app->state.view = APP_VIEW_MAIN_MENU;
                app->state.capture_title[0] = '\0';
                app->state.capture_cursor = 0;
                snprintf(app->state.message, sizeof(app->state.message), "%s", "Captured");
                app->needs_redraw = 1;
            }
            return;
        }
        if (key >= 32 && key <= 126) {
            capture_insert_char(app, (char)key);
        }
        return;
    }

    if (key == TERMINAL_KEY_UP) {
        app->state.selected_menu_index = (app->state.selected_menu_index + 2) % 3;
    } else if (key == TERMINAL_KEY_DOWN) {
        app->state.selected_menu_index = (app->state.selected_menu_index + 1) % 3;
    } else if (key >= '1' && key <= '3') {
        app->state.selected_menu_index = key - '1';
    } else if (key == TERMINAL_KEY_ENTER) {
        if (app->state.selected_menu_index == 0) {
            app->state.view = APP_VIEW_CAPTURE;
            app->state.capture_title[0] = '\0';
            app->state.capture_cursor = 0;
            app->needs_redraw = 1;
        } else if (app->state.selected_menu_index == 2) {
            app->should_quit = 1;
        }
    }

    if (previous_selection != app->state.selected_menu_index) {
        app->needs_redraw = 1;
    }
}
