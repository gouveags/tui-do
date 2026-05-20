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

static TodoIndexEntry *app_selected_entry(App *app) {
    if (
        app->state.selected_inbox_index < 0 ||
        app->state.selected_inbox_index >= (int)app->state.inbox.entry_count
    ) {
        return NULL;
    }

    return &app->state.inbox.entries[app->state.selected_inbox_index];
}

static void note_insert_char(App *app, char value) {
    size_t length = strlen(app->state.note_text);
    int cursor = app->state.note_cursor;

    if (length + 1 >= sizeof(app->state.note_text)) {
        return;
    }
    if (cursor < 0 || cursor > (int)length) {
        cursor = (int)length;
    }

    memmove(
        app->state.note_text + cursor + 1,
        app->state.note_text + cursor,
        length - (size_t)cursor + 1
    );
    app->state.note_text[cursor] = value;
    app->state.note_cursor = cursor + 1;
    app->needs_redraw = 1;
}

static void note_backspace(App *app) {
    size_t length = strlen(app->state.note_text);
    int cursor = app->state.note_cursor;

    if (cursor <= 0 || length == 0) {
        return;
    }
    if (cursor > (int)length) {
        cursor = (int)length;
    }

    memmove(
        app->state.note_text + cursor - 1,
        app->state.note_text + cursor,
        length - (size_t)cursor + 1
    );
    app->state.note_cursor = cursor - 1;
    app->needs_redraw = 1;
}

static void note_cursor_shape(const char *text, int cursor, int *line_start, int *column, int *line_end) {
    int length = (int)strlen(text);
    int start = 0;
    int end;

    if (cursor < 0) {
        cursor = 0;
    }
    if (cursor > length) {
        cursor = length;
    }

    for (int i = 0; i < cursor; i++) {
        if (text[i] == '\n') {
            start = i + 1;
        }
    }

    end = start;
    while (text[end] != '\0' && text[end] != '\n') {
        end++;
    }

    *line_start = start;
    *column = cursor - start;
    *line_end = end;
}

static int note_cursor_for_line_column(const char *text, int line_start, int desired_column) {
    int line_end = line_start;
    int line_length;

    while (text[line_end] != '\0' && text[line_end] != '\n') {
        line_end++;
    }

    line_length = line_end - line_start;
    if (desired_column > line_length) {
        desired_column = line_length;
    }
    if (desired_column < 0) {
        desired_column = 0;
    }

    return line_start + desired_column;
}

static void note_move_vertical(App *app, int direction) {
    int line_start;
    int column;
    int line_end;
    int target_start;

    note_cursor_shape(app->state.note_text, app->state.note_cursor, &line_start, &column, &line_end);

    if (direction < 0) {
        if (line_start == 0) {
            return;
        }

        target_start = line_start - 1;
        while (target_start > 0 && app->state.note_text[target_start - 1] != '\n') {
            target_start--;
        }
    } else {
        if (app->state.note_text[line_end] == '\0') {
            return;
        }

        target_start = line_end + 1;
    }

    app->state.note_cursor = note_cursor_for_line_column(app->state.note_text, target_start, column);
    app->needs_redraw = 1;
}

static void app_open_note_editor(App *app) {
    TodoIndexEntry *entry = app_selected_entry(app);

    if (entry == NULL) {
        return;
    }
    if (storage_load_todo_note(app->storage_root, entry->id, app->state.note_text, sizeof(app->state.note_text)) != 0) {
        return;
    }

    app->state.note_cursor = (int)strlen(app->state.note_text);
    app->state.note_scroll_line = 0;
    app->state.view = APP_VIEW_NOTE_EDITOR;
    app->needs_redraw = 1;
}

static void app_save_note_editor(App *app) {
    TodoIndexEntry *entry = app_selected_entry(app);

    if (entry == NULL) {
        return;
    }
    if (storage_save_todo_note(app->storage_root, entry->id, app->state.note_text) == 0) {
        snprintf(app->state.message, sizeof(app->state.message), "%s", "Saved");
        app->state.view = APP_VIEW_DETAIL;
        app->needs_redraw = 1;
    }
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
    if (app->state.view == APP_VIEW_NOTE_EDITOR) {
        app->state.view = APP_VIEW_DETAIL;
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

static void app_scroll_note(App *app, int delta) {
    app->state.note_scroll_line += delta;
    if (app->state.note_scroll_line < 0) {
        app->state.note_scroll_line = 0;
    }
    app->needs_redraw = 1;
}

static void app_scroll_detail(App *app, int delta) {
    app->state.detail_scroll_line += delta;
    if (app->state.detail_scroll_line < 0) {
        app->state.detail_scroll_line = 0;
    }
    app->needs_redraw = 1;
}

void app_handle_key(App *app, int key) {
    int previous_selection = app->state.selected_menu_index;

    if (key == TERMINAL_KEY_CTRL_C) {
        app->should_quit = 1;
        return;
    }

    if (key == TERMINAL_KEY_ESCAPE) {
        app_go_back(app);
        return;
    }

    if (app->state.view == APP_VIEW_NOTE_EDITOR) {
        if (key == TERMINAL_KEY_PAGE_UP || key == TERMINAL_KEY_MOUSE_WHEEL_UP) {
            app_scroll_note(app, -3);
            return;
        }
        if (key == TERMINAL_KEY_PAGE_DOWN || key == TERMINAL_KEY_MOUSE_WHEEL_DOWN) {
            app_scroll_note(app, 3);
            return;
        }
        if (key == TERMINAL_KEY_CTRL_S) {
            app_save_note_editor(app);
            return;
        }
        if (key == TERMINAL_KEY_SHIFT_ENTER || key == TERMINAL_KEY_ENTER) {
            note_insert_char(app, '\n');
            return;
        }
        if (key == TERMINAL_KEY_BACKSPACE) {
            note_backspace(app);
            return;
        }
        if (key == TERMINAL_KEY_LEFT) {
            if (app->state.note_cursor > 0) {
                app->state.note_cursor--;
                app->needs_redraw = 1;
            }
            return;
        }
        if (key == TERMINAL_KEY_RIGHT) {
            if (app->state.note_cursor < (int)strlen(app->state.note_text)) {
                app->state.note_cursor++;
                app->needs_redraw = 1;
            }
            return;
        }
        if (key == TERMINAL_KEY_UP) {
            note_move_vertical(app, -1);
            return;
        }
        if (key == TERMINAL_KEY_DOWN) {
            note_move_vertical(app, 1);
            return;
        }
        if (key >= 32 && key <= 126) {
            note_insert_char(app, (char)key);
        }
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
        if (key == 'q') {
            app->should_quit = 1;
            return;
        }
        if (key == 'm' || key == 'M') {
            app_return_to_main_menu(app);
            return;
        }
        app_handle_inbox_key(app, key);
        return;
    }

    if (app->state.view == APP_VIEW_DETAIL) {
        if (key == TERMINAL_KEY_PAGE_UP || key == TERMINAL_KEY_MOUSE_WHEEL_UP) {
            app_scroll_detail(app, -3);
            return;
        }
        if (key == TERMINAL_KEY_PAGE_DOWN || key == TERMINAL_KEY_MOUSE_WHEEL_DOWN) {
            app_scroll_detail(app, 3);
            return;
        }
        if (key == 'q') {
            app->should_quit = 1;
            return;
        }
        if (key == 'm' || key == 'M') {
            app_return_to_main_menu(app);
            return;
        }
        if (key != 'e' && key != 'E') {
            return;
        }
        app_open_note_editor(app);
        return;
    }

    if (key == 'q') {
        app->should_quit = 1;
        return;
    }
    if (key == 'm' || key == 'M') {
        app_return_to_main_menu(app);
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
