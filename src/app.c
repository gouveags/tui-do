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
            .selected_inbox_index = 0,
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

static void app_sort_inbox_newest_first(App *app) {
    for (size_t i = 0; i < app->state.inbox.entry_count; i++) {
        for (size_t j = i + 1; j < app->state.inbox.entry_count; j++) {
            TodoIndexEntry *left = &app->state.inbox.entries[i];
            TodoIndexEntry *right = &app->state.inbox.entries[j];
            if (right->created_at > left->created_at) {
                TodoIndexEntry tmp = *left;
                *left = *right;
                *right = tmp;
            }
        }
    }
}

static void app_open_inbox(App *app) {
    storage_load_index(app->storage_root, &app->state.inbox);
    app_sort_inbox_newest_first(app);
    app->state.selected_inbox_index = 0;
    app->state.view = APP_VIEW_INBOX;
    app->needs_redraw = 1;
}

static void app_start_capture(App *app) {
    app->state.view = APP_VIEW_CAPTURE;
    app->state.capture_title[0] = '\0';
    app->state.capture_cursor = 0;
    app->needs_redraw = 1;
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

static void app_return_to_main_menu(App *app) {
    if (app->state.view != APP_VIEW_MAIN_MENU || app->state.capture_title[0] != '\0') {
        app->state.view = APP_VIEW_MAIN_MENU;
        app->state.capture_title[0] = '\0';
        app->state.capture_cursor = 0;
        app->needs_redraw = 1;
    }
}

static void app_go_back(App *app) {
    if (app->state.view == APP_VIEW_DETAIL) {
        app->state.view = APP_VIEW_INBOX;
        app->needs_redraw = 1;
        return;
    }

    app_return_to_main_menu(app);
}

static void app_activate_menu_item(App *app, int index) {
    app->state.selected_menu_index = index;

    if (index == 0) {
        app_start_capture(app);
    } else if (index == 1) {
        app_open_inbox(app);
    } else if (index == 2) {
        app->should_quit = 1;
    } else {
        app->needs_redraw = 1;
    }
}

static void app_save_capture(App *app) {
    Todo todo = {
        .created_at = app->now,
        .updated_at = app->now,
        .item_count = 0,
    };

    snprintf(todo.id, sizeof(todo.id), "entry-%d", app->next_id++);
    snprintf(todo.title, sizeof(todo.title), "%s", app->state.capture_title);
    if (storage_save_todo(app->storage_root, &todo) == 0) {
        app->state.capture_title[0] = '\0';
        app->state.capture_cursor = 0;
        snprintf(app->state.message, sizeof(app->state.message), "%s", "Captured");
        app_open_inbox(app);
    }
}

static void app_handle_inbox_key(App *app, int key) {
    size_t entry_count = app->state.inbox.entry_count;

    if (key == 'n' || key == 'N' || key == '1') {
        app_start_capture(app);
        return;
    }
    if (entry_count == 0) {
        return;
    }
    if (key == TERMINAL_KEY_UP) {
        app->state.selected_inbox_index = (app->state.selected_inbox_index + (int)entry_count - 1) % (int)entry_count;
        app->needs_redraw = 1;
        return;
    }
    if (key == TERMINAL_KEY_DOWN) {
        app->state.selected_inbox_index = (app->state.selected_inbox_index + 1) % (int)entry_count;
        app->needs_redraw = 1;
        return;
    }
    if (key == TERMINAL_KEY_ENTER) {
        app->state.view = APP_VIEW_DETAIL;
        app->needs_redraw = 1;
    }
}

void app_handle_key(App *app, int key) {
    int previous_selection = app->state.selected_menu_index;

    if (key == TERMINAL_KEY_CTRL_C || key == 'q') {
        app->should_quit = 1;
        return;
    }

    if (key == 'm' || key == 'M') {
        app_return_to_main_menu(app);
        return;
    }
    if (key == TERMINAL_KEY_ESCAPE) {
        app_go_back(app);
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

            app_save_capture(app);
            return;
        }
        if (key >= 32 && key <= 126) {
            capture_insert_char(app, (char)key);
        }
        return;
    }

    if (app->state.view == APP_VIEW_INBOX) {
        app_handle_inbox_key(app, key);
        return;
    }

    if (app->state.view == APP_VIEW_DETAIL) {
        return;
    }

    if (key == TERMINAL_KEY_UP) {
        app->state.selected_menu_index = (app->state.selected_menu_index + 2) % 3;
    } else if (key == TERMINAL_KEY_DOWN) {
        app->state.selected_menu_index = (app->state.selected_menu_index + 1) % 3;
    } else if (key >= '1' && key <= '3') {
        app_activate_menu_item(app, key - '1');
        return;
    } else if (key == TERMINAL_KEY_ENTER) {
        app_activate_menu_item(app, app->state.selected_menu_index);
        return;
    }

    if (previous_selection != app->state.selected_menu_index) {
        app->needs_redraw = 1;
    }
}
