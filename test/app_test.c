#include "test.h"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../src/app.h"
#include "../src/storage.h"

static int app_test_file_exists(const char *path) {
    struct stat info;
    return stat(path, &info) == 0;
}

SCENARIO(app_marks_itself_for_redraw_when_the_terminal_resizes) {
    App app = app_create((TerminalSize) { .width = 160, .height = 50 });
    app.needs_redraw = 0;

    GIVEN("an app already rendered at a large terminal size");

    WHEN("the terminal is resized smaller");
    app_handle_resize(&app, (TerminalSize) { .width = 72, .height = 20 });

    THEN("the app stores the exact new size and schedules a redraw");
    EXPECT_INT_EQ(app.terminal.width, 72);
    EXPECT_INT_EQ(app.terminal.height, 20);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(app_does_not_redraw_when_the_terminal_size_is_unchanged) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.needs_redraw = 0;

    GIVEN("an app rendered at the current terminal size");

    WHEN("resize handling observes the same size again");
    app_handle_resize(&app, (TerminalSize) { .width = 100, .height = 30 });

    THEN("no extra redraw is scheduled");
    EXPECT_INT_EQ(app.needs_redraw, 0);
}

SCENARIO(app_waits_for_resize_to_stabilize_before_redrawing) {
    App app = app_create((TerminalSize) { .width = 120, .height = 40 });
    app.needs_redraw = 0;

    GIVEN("a resize reports an intermediate terminal size");

    WHEN("the app observes that size for the first time");
    app_observe_terminal_size(&app, (TerminalSize) { .width = 90, .height = 24 });

    THEN("the current layout size is not replaced yet");
    EXPECT_INT_EQ(app.terminal.width, 120);
    EXPECT_INT_EQ(app.terminal.height, 40);
    EXPECT_INT_EQ(app.needs_redraw, 0);

    WHEN("the same size is observed again");
    app_observe_terminal_size(&app, (TerminalSize) { .width = 90, .height = 24 });

    THEN("the resize is treated as stable and the app redraws");
    EXPECT_INT_EQ(app.terminal.width, 90);
    EXPECT_INT_EQ(app.terminal.height, 24);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(app_uses_the_latest_size_after_noisy_resize_events) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.needs_redraw = 0;

    GIVEN("multiple transient sizes are observed during one resize");

    WHEN("the final full-screen size is the one that stabilizes");
    app_observe_terminal_size(&app, (TerminalSize) { .width = 72, .height = 20 });
    app_observe_terminal_size(&app, (TerminalSize) { .width = 160, .height = 48 });
    app_observe_terminal_size(&app, (TerminalSize) { .width = 160, .height = 48 });

    THEN("the app adopts the latest stable size, not the first transient one");
    EXPECT_INT_EQ(app.terminal.width, 160);
    EXPECT_INT_EQ(app.terminal.height, 48);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(menu_navigation_wraps_with_arrow_keys) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.needs_redraw = 0;

    GIVEN("the first main menu item is selected");

    WHEN("the user navigates upward");
    app_handle_key(&app, TERMINAL_KEY_UP);

    THEN("selection wraps to the last menu item and redraws");
    EXPECT_INT_EQ(app.state.selected_menu_index, 2);
    EXPECT_INT_EQ(app.needs_redraw, 1);

    WHEN("the user navigates downward");
    app.needs_redraw = 0;
    app_handle_key(&app, TERMINAL_KEY_DOWN);

    THEN("selection wraps back to the first menu item");
    EXPECT_INT_EQ(app.state.selected_menu_index, 0);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(menu_quick_select_keys_jump_to_items) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.needs_redraw = 0;

    GIVEN("the main menu is open");

    WHEN("the user presses 2");
    app_handle_key(&app, '2');

    THEN("the second menu item is selected");
    EXPECT_INT_EQ(app.state.selected_menu_index, 1);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(menu_number_keys_activate_their_items_without_enter) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.needs_redraw = 0;

    GIVEN("the main menu is open");

    WHEN("the user presses 1");
    app_handle_key(&app, '1');

    THEN("the Create action opens capture immediately");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_CAPTURE);
    EXPECT_INT_EQ(app.needs_redraw, 1);

    WHEN("the user returns to main and presses 3");
    app.state.view = APP_VIEW_MAIN_MENU;
    app.needs_redraw = 0;
    app_handle_key(&app, '3');

    THEN("the Quit action requests termination immediately");
    EXPECT_INT_EQ(app.should_quit, 1);
}

SCENARIO(menu_quit_keys_request_termination) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });

    GIVEN("the main menu is open");

    WHEN("the user presses Ctrl+C");
    app_handle_key(&app, TERMINAL_KEY_CTRL_C);

    THEN("the app requests termination");
    EXPECT_INT_EQ(app.should_quit, 1);
}

SCENARIO(selecting_create_opens_the_capture_view) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.needs_redraw = 0;

    GIVEN("the Create menu item is selected");

    WHEN("the user presses Enter");
    app_handle_key(&app, TERMINAL_KEY_ENTER);

    THEN("the app opens the capture view");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_CAPTURE);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(capture_view_edits_the_title_buffer) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_CAPTURE;

    GIVEN("the capture view is open");

    WHEN("the user types and corrects a title");
    app_handle_key(&app, 'B');
    app_handle_key(&app, 'u');
    app_handle_key(&app, 'g');
    app_handle_key(&app, TERMINAL_KEY_BACKSPACE);
    app_handle_key(&app, 'y');

    THEN("the title buffer reflects the edit");
    EXPECT_TRUE(strcmp(app.state.capture_title, "Buy") == 0);
}

SCENARIO(capture_view_treats_action_letters_as_text) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_CAPTURE;

    GIVEN("the capture title field is focused");

    WHEN("the user types letters that are actions elsewhere");
    app_handle_key(&app, 'q');
    app_handle_key(&app, 'm');

    THEN("the letters are inserted instead of triggering global actions");
    EXPECT_TRUE(strcmp(app.state.capture_title, "qm") == 0);
    EXPECT_INT_EQ(app.should_quit, 0);
    EXPECT_INT_EQ(app.state.view, APP_VIEW_CAPTURE);
}

SCENARIO(capture_view_moves_the_cursor_and_inserts_at_cursor) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_CAPTURE;

    GIVEN("the capture view has text");
    app_handle_key(&app, 'A');
    app_handle_key(&app, 'C');

    WHEN("the user moves left and types");
    app_handle_key(&app, TERMINAL_KEY_LEFT);
    app_handle_key(&app, 'B');

    THEN("the character is inserted at the cursor");
    EXPECT_TRUE(strcmp(app.state.capture_title, "ABC") == 0);
    EXPECT_INT_EQ(app.state.capture_cursor, 2);

    WHEN("the user moves right");
    app_handle_key(&app, TERMINAL_KEY_RIGHT);

    THEN("the cursor moves toward the end of the input");
    EXPECT_INT_EQ(app.state.capture_cursor, 3);
}

SCENARIO(capture_view_backspace_removes_before_cursor) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_CAPTURE;

    GIVEN("the cursor is in the middle of the input");
    app_handle_key(&app, 'A');
    app_handle_key(&app, 'B');
    app_handle_key(&app, 'C');
    app_handle_key(&app, TERMINAL_KEY_LEFT);

    WHEN("the user presses backspace");
    app_handle_key(&app, TERMINAL_KEY_BACKSPACE);

    THEN("the character before the cursor is removed");
    EXPECT_TRUE(strcmp(app.state.capture_title, "AC") == 0);
    EXPECT_INT_EQ(app.state.capture_cursor, 1);
}

SCENARIO(capture_view_does_not_save_empty_titles) {
    App app = app_create_with_storage((TerminalSize) { .width = 100, .height = 30 }, "build/app-capture-empty");
    TodoIndex index = {0};

    GIVEN("the capture view is open with an empty title");
    system("rm -rf build/app-capture-empty");
    app.state.view = APP_VIEW_CAPTURE;

    WHEN("the user presses Enter");
    app_handle_key(&app, TERMINAL_KEY_ENTER);

    THEN("the app remains in capture and no todo is saved");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_CAPTURE);
    EXPECT_INT_EQ(storage_load_index("build/app-capture-empty", &index), 0);
    EXPECT_INT_EQ((int)index.entry_count, 0);
}

SCENARIO(capture_view_saves_a_new_inbox_todo) {
    App app = app_create_with_storage((TerminalSize) { .width = 100, .height = 30 }, "build/app-capture-save");
    TodoIndex index = {0};
    char note_path[TODO_PATH_MAX];

    GIVEN("the capture view has a title");
    system("rm -rf build/app-capture-save");
    app.state.view = APP_VIEW_CAPTURE;
    app.now = 1234;
    app_handle_key(&app, 'F');
    app_handle_key(&app, 'i');
    app_handle_key(&app, 'x');

    WHEN("the user presses Enter");
    app_handle_key(&app, TERMINAL_KEY_ENTER);

    THEN("the todo is saved and the app opens the inbox");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_INBOX);
    EXPECT_TRUE(strcmp(app.state.capture_title, "") == 0);
    EXPECT_INT_EQ((int)app.state.inbox.entry_count, 1);
    EXPECT_TRUE(strcmp(app.state.inbox.entries[0].title, "Fix") == 0);
    EXPECT_INT_EQ(storage_load_index("build/app-capture-save", &index), 0);
    EXPECT_INT_EQ((int)index.entry_count, 1);
    EXPECT_TRUE(strcmp(index.entries[0].title, "Fix") == 0);
    EXPECT_INT_EQ((int)index.entries[0].created_at, 1234);
    EXPECT_INT_EQ((int)index.entries[0].updated_at, 1234);
    EXPECT_INT_EQ(storage_todo_note_path("build/app-capture-save", index.entries[0].id, note_path, sizeof(note_path)), 0);
    EXPECT_TRUE(app_test_file_exists(note_path));
}

SCENARIO(load_menu_opens_the_inbox_newest_first) {
    App app = app_create_with_storage((TerminalSize) { .width = 100, .height = 30 }, "build/app-inbox-load");
    Todo older = {
        .id = "entry-older",
        .title = "Older note",
        .created_at = 10,
        .updated_at = 10,
    };
    Todo newer = {
        .id = "entry-newer",
        .title = "Newer note",
        .created_at = 20,
        .updated_at = 20,
    };

    GIVEN("two notebook entries have been stored");
    system("rm -rf build/app-inbox-load");
    storage_save_todo("build/app-inbox-load", &older);
    storage_save_todo("build/app-inbox-load", &newer);

    WHEN("the user activates Load a to-do");
    app_handle_key(&app, '2');

    THEN("the inbox opens with the newest entry first");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_INBOX);
    EXPECT_INT_EQ((int)app.state.inbox.entry_count, 2);
    EXPECT_TRUE(strcmp(app.state.inbox.entries[0].title, "Newer note") == 0);
    EXPECT_TRUE(strcmp(app.state.inbox.entries[1].title, "Older note") == 0);
    EXPECT_INT_EQ(app.state.selected_inbox_index, 0);
}

SCENARIO(inbox_navigation_opens_the_selected_detail) {
    App app = app_create_with_storage((TerminalSize) { .width = 100, .height = 30 }, "build/app-inbox-nav");
    Todo first = {
        .id = "entry-first",
        .title = "First",
        .created_at = 10,
        .updated_at = 10,
    };
    Todo second = {
        .id = "entry-second",
        .title = "Second",
        .created_at = 20,
        .updated_at = 20,
    };

    GIVEN("the inbox is open with saved entries");
    system("rm -rf build/app-inbox-nav");
    storage_save_todo("build/app-inbox-nav", &first);
    storage_save_todo("build/app-inbox-nav", &second);
    app_handle_key(&app, '2');

    WHEN("the user moves down and opens an entry");
    app_handle_key(&app, TERMINAL_KEY_DOWN);
    app_handle_key(&app, TERMINAL_KEY_ENTER);

    THEN("the detail screen opens for the selected entry");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_DETAIL);
    EXPECT_INT_EQ(app.state.selected_inbox_index, 1);
    EXPECT_TRUE(strcmp(app.state.inbox.entries[app.state.selected_inbox_index].title, "First") == 0);
}

SCENARIO(inbox_create_bind_starts_a_new_capture) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_INBOX;

    GIVEN("the inbox is open");

    WHEN("the user presses n");
    app_handle_key(&app, 'n');

    THEN("a new capture starts");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_CAPTURE);
    EXPECT_TRUE(strcmp(app.state.capture_title, "") == 0);
    EXPECT_INT_EQ(app.state.capture_cursor, 0);
}

SCENARIO(escape_returns_from_detail_to_inbox_before_main_menu) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_DETAIL;
    app.state.inbox.entry_count = 1;
    snprintf(app.state.inbox.entries[0].title, sizeof(app.state.inbox.entries[0].title), "%s", "Open item");

    GIVEN("an item detail is open");

    WHEN("the user presses Escape");
    app_handle_key(&app, TERMINAL_KEY_ESCAPE);

    THEN("the app returns to the previous inbox screen");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_INBOX);

    WHEN("the user presses Escape again");
    app_handle_key(&app, TERMINAL_KEY_ESCAPE);

    THEN("the app returns to the main menu");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_MAIN_MENU);
}

SCENARIO(detail_edit_bind_records_the_selected_note_path) {
    App app = app_create_with_storage((TerminalSize) { .width = 100, .height = 30 }, "build/app-detail-edit");
    Todo todo = {
        .id = "entry-edit",
        .title = "Edit this note",
        .created_at = 10,
        .updated_at = 10,
    };

    GIVEN("an entry detail is open");
    system("rm -rf build/app-detail-edit");
    storage_save_todo("build/app-detail-edit", &todo);
    storage_save_todo_note("build/app-detail-edit", "entry-edit", "# Edit this note\n\nOriginal text\n");
    app_handle_key(&app, '2');
    app_handle_key(&app, TERMINAL_KEY_ENTER);

    WHEN("the user presses e");
    app_handle_key(&app, 'e');

    THEN("the built-in note editor opens with the note content");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_NOTE_EDITOR);
    EXPECT_TRUE(strcmp(app.state.note_text, "# Edit this note\n\nOriginal text\n") == 0);
    EXPECT_INT_EQ(app.state.note_cursor, (int)strlen(app.state.note_text));
}

SCENARIO(note_editor_edits_multiple_lines_and_saves) {
    App app = app_create_with_storage((TerminalSize) { .width = 100, .height = 30 }, "build/app-note-editor");
    Todo todo = {
        .id = "entry-editor",
        .title = "Editor note",
        .created_at = 10,
        .updated_at = 10,
    };
    char note[TODO_NOTE_MAX];

    GIVEN("the built-in note editor is open");
    system("rm -rf build/app-note-editor");
    storage_save_todo("build/app-note-editor", &todo);
    app_handle_key(&app, '2');
    app_handle_key(&app, TERMINAL_KEY_ENTER);
    app_handle_key(&app, 'e');

    WHEN("the user appends text, inserts a Shift+Enter line break, and saves");
    app_handle_key(&app, 'N');
    app_handle_key(&app, 'e');
    app_handle_key(&app, 'x');
    app_handle_key(&app, 't');
    app_handle_key(&app, TERMINAL_KEY_SHIFT_ENTER);
    app_handle_key(&app, 'L');
    app_handle_key(&app, 'i');
    app_handle_key(&app, 'n');
    app_handle_key(&app, 'e');
    app_handle_key(&app, TERMINAL_KEY_CTRL_S);

    THEN("the note is saved and the app returns to detail");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_DETAIL);
    EXPECT_INT_EQ(storage_load_todo_note("build/app-note-editor", "entry-editor", note, sizeof(note)), 0);
    EXPECT_TRUE(strstr(note, "Next\nLine") != NULL);
}

SCENARIO(note_editor_treats_action_letters_as_text) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_NOTE_EDITOR;

    GIVEN("the built-in note editor is focused");

    WHEN("the user types letters that are actions elsewhere");
    app_handle_key(&app, 'q');
    app_handle_key(&app, 'm');
    app_handle_key(&app, 'e');

    THEN("the letters are inserted into the note");
    EXPECT_TRUE(strcmp(app.state.note_text, "qme") == 0);
    EXPECT_INT_EQ(app.should_quit, 0);
    EXPECT_INT_EQ(app.state.view, APP_VIEW_NOTE_EDITOR);
}

SCENARIO(note_editor_moves_vertically_between_lines) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_NOTE_EDITOR;
    snprintf(app.state.note_text, sizeof(app.state.note_text), "%s", "Alpha\nBeta\nGamma");
    app.state.note_cursor = (int)strlen("Alpha\nBe");

    GIVEN("the note editor cursor is in the middle line");

    WHEN("the user presses up");
    app_handle_key(&app, TERMINAL_KEY_UP);

    THEN("the cursor moves to the same visual column on the previous line");
    EXPECT_INT_EQ(app.state.note_cursor, (int)strlen("Al"));
    EXPECT_INT_EQ(app.needs_redraw, 1);

    WHEN("the user presses down");
    app.needs_redraw = 0;
    app_handle_key(&app, TERMINAL_KEY_DOWN);

    THEN("the cursor returns to that column on the next line");
    EXPECT_INT_EQ(app.state.note_cursor, (int)strlen("Alpha\nBe"));
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(note_editor_vertical_movement_clamps_to_shorter_lines) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_NOTE_EDITOR;
    snprintf(app.state.note_text, sizeof(app.state.note_text), "%s", "Longer\nYo\nLongest");
    app.state.note_cursor = (int)strlen("Longer");

    GIVEN("the note editor cursor is beyond the length of a neighboring line");

    WHEN("the user presses down");
    app_handle_key(&app, TERMINAL_KEY_DOWN);

    THEN("the cursor lands at the end of the shorter line");
    EXPECT_INT_EQ(app.state.note_cursor, (int)strlen("Longer\nYo"));
}

SCENARIO(note_editor_vertical_movement_stops_at_file_edges) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_NOTE_EDITOR;
    snprintf(app.state.note_text, sizeof(app.state.note_text), "%s", "Top\nBottom");
    app.state.note_cursor = 1;

    GIVEN("the note editor cursor is on the first line");

    WHEN("the user presses up");
    app_handle_key(&app, TERMINAL_KEY_UP);

    THEN("the cursor stays on the first line");
    EXPECT_INT_EQ(app.state.note_cursor, 1);

    WHEN("the user moves to the last line and presses down");
    app.state.note_cursor = (int)strlen("Top\nBo");
    app_handle_key(&app, TERMINAL_KEY_DOWN);

    THEN("the cursor stays on the last line");
    EXPECT_INT_EQ(app.state.note_cursor, (int)strlen("Top\nBo"));
}

SCENARIO(note_editor_scrolls_with_page_and_mouse_wheel) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_NOTE_EDITOR;

    GIVEN("the note editor contains more text than fits on screen");

    WHEN("the user scrolls down with the page key and mouse wheel");
    app_handle_key(&app, TERMINAL_KEY_PAGE_DOWN);
    app_handle_key(&app, TERMINAL_KEY_MOUSE_WHEEL_DOWN);

    THEN("the editor records a scroll offset");
    EXPECT_INT_EQ(app.state.note_scroll_line, 6);
    EXPECT_INT_EQ(app.needs_redraw, 1);

    WHEN("the user scrolls back above the top");
    app_handle_key(&app, TERMINAL_KEY_PAGE_UP);
    app_handle_key(&app, TERMINAL_KEY_PAGE_UP);
    app_handle_key(&app, TERMINAL_KEY_MOUSE_WHEEL_UP);

    THEN("the editor clamps scrolling at the first line");
    EXPECT_INT_EQ(app.state.note_scroll_line, 0);
}

SCENARIO(detail_view_scrolls_with_page_and_mouse_wheel) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_DETAIL;

    GIVEN("the markdown detail view contains more text than fits on screen");

    WHEN("the user scrolls down");
    app_handle_key(&app, TERMINAL_KEY_PAGE_DOWN);
    app_handle_key(&app, TERMINAL_KEY_MOUSE_WHEEL_DOWN);

    THEN("the detail view records a scroll offset");
    EXPECT_INT_EQ(app.state.detail_scroll_line, 6);

    WHEN("the user scrolls back above the top");
    app_handle_key(&app, TERMINAL_KEY_PAGE_UP);
    app_handle_key(&app, TERMINAL_KEY_PAGE_UP);
    app_handle_key(&app, TERMINAL_KEY_MOUSE_WHEEL_UP);

    THEN("the detail view clamps scrolling at the first line");
    EXPECT_INT_EQ(app.state.detail_scroll_line, 0);
}

SCENARIO(escape_goes_back_to_the_previous_screen) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_CAPTURE;
    app.needs_redraw = 0;
    app_handle_key(&app, 'A');

    GIVEN("the capture view is open with unsaved text");

    WHEN("the user presses Escape");
    app_handle_key(&app, TERMINAL_KEY_ESCAPE);

    THEN("the app returns to the main menu and clears capture input");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_MAIN_MENU);
    EXPECT_TRUE(strcmp(app.state.capture_title, "") == 0);
    EXPECT_INT_EQ(app.state.capture_cursor, 0);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(main_menu_key_returns_to_main_from_any_screen) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.state.view = APP_VIEW_INBOX;
    app.needs_redraw = 0;

    GIVEN("the user is on a non-typing screen");

    WHEN("the user presses m");
    app_handle_key(&app, 'm');

    THEN("the app returns to the main menu");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_MAIN_MENU);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

int main(void) {
    RUN_SCENARIO(app_marks_itself_for_redraw_when_the_terminal_resizes);
    RUN_SCENARIO(app_does_not_redraw_when_the_terminal_size_is_unchanged);
    RUN_SCENARIO(app_waits_for_resize_to_stabilize_before_redrawing);
    RUN_SCENARIO(app_uses_the_latest_size_after_noisy_resize_events);
    RUN_SCENARIO(menu_navigation_wraps_with_arrow_keys);
    RUN_SCENARIO(menu_quick_select_keys_jump_to_items);
    RUN_SCENARIO(menu_number_keys_activate_their_items_without_enter);
    RUN_SCENARIO(menu_quit_keys_request_termination);
    RUN_SCENARIO(selecting_create_opens_the_capture_view);
    RUN_SCENARIO(capture_view_edits_the_title_buffer);
    RUN_SCENARIO(capture_view_treats_action_letters_as_text);
    RUN_SCENARIO(capture_view_moves_the_cursor_and_inserts_at_cursor);
    RUN_SCENARIO(capture_view_backspace_removes_before_cursor);
    RUN_SCENARIO(capture_view_does_not_save_empty_titles);
    RUN_SCENARIO(capture_view_saves_a_new_inbox_todo);
    RUN_SCENARIO(load_menu_opens_the_inbox_newest_first);
    RUN_SCENARIO(inbox_navigation_opens_the_selected_detail);
    RUN_SCENARIO(inbox_create_bind_starts_a_new_capture);
    RUN_SCENARIO(escape_returns_from_detail_to_inbox_before_main_menu);
    RUN_SCENARIO(detail_edit_bind_records_the_selected_note_path);
    RUN_SCENARIO(note_editor_edits_multiple_lines_and_saves);
    RUN_SCENARIO(note_editor_treats_action_letters_as_text);
    RUN_SCENARIO(note_editor_moves_vertically_between_lines);
    RUN_SCENARIO(note_editor_vertical_movement_clamps_to_shorter_lines);
    RUN_SCENARIO(note_editor_vertical_movement_stops_at_file_edges);
    RUN_SCENARIO(note_editor_scrolls_with_page_and_mouse_wheel);
    RUN_SCENARIO(detail_view_scrolls_with_page_and_mouse_wheel);
    RUN_SCENARIO(escape_goes_back_to_the_previous_screen);
    RUN_SCENARIO(main_menu_key_returns_to_main_from_any_screen);

    return finish_tests();
}
