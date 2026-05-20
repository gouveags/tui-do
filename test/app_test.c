#include "test.h"

#include <string.h>
#include <stdlib.h>

#include "../src/app.h"
#include "../src/storage.h"

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

    GIVEN("the capture view has a title");
    system("rm -rf build/app-capture-save");
    app.state.view = APP_VIEW_CAPTURE;
    app.now = 1234;
    app_handle_key(&app, 'F');
    app_handle_key(&app, 'i');
    app_handle_key(&app, 'x');

    WHEN("the user presses Enter");
    app_handle_key(&app, TERMINAL_KEY_ENTER);

    THEN("the todo is saved and the app returns to the main menu");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_MAIN_MENU);
    EXPECT_TRUE(strcmp(app.state.capture_title, "") == 0);
    EXPECT_INT_EQ(storage_load_index("build/app-capture-save", &index), 0);
    EXPECT_INT_EQ((int)index.entry_count, 1);
    EXPECT_TRUE(strcmp(index.entries[0].title, "Fix") == 0);
    EXPECT_INT_EQ((int)index.entries[0].created_at, 1234);
    EXPECT_INT_EQ((int)index.entries[0].updated_at, 1234);
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
    app.state.view = APP_VIEW_CAPTURE;
    app.needs_redraw = 0;
    app_handle_key(&app, 'A');

    GIVEN("the user is away from the main menu");

    WHEN("the user presses m");
    app_handle_key(&app, 'm');

    THEN("the app returns to the main menu");
    EXPECT_INT_EQ(app.state.view, APP_VIEW_MAIN_MENU);
    EXPECT_TRUE(strcmp(app.state.capture_title, "") == 0);
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
    RUN_SCENARIO(capture_view_moves_the_cursor_and_inserts_at_cursor);
    RUN_SCENARIO(capture_view_backspace_removes_before_cursor);
    RUN_SCENARIO(capture_view_does_not_save_empty_titles);
    RUN_SCENARIO(capture_view_saves_a_new_inbox_todo);
    RUN_SCENARIO(escape_goes_back_to_the_previous_screen);
    RUN_SCENARIO(main_menu_key_returns_to_main_from_any_screen);

    return finish_tests();
}
