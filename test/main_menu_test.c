#define _POSIX_C_SOURCE 200112L

#include "test.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CLAY_IMPLEMENTATION
#include "../vendor/clay/clay.h"

#include "../src/terminal.h"
#include "../src/ui.h"

static void handle_clay_error(Clay_ErrorData error_data) {
    fprintf(stderr, "Clay error: %.*s\n", error_data.errorText.length, error_data.errorText.chars);
    test_failures++;
}

static Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data) {
    (void)config;
    (void)user_data;

    return (Clay_Dimensions) {
        .width = (float)text.length,
        .height = 1.0f,
    };
}

static void setup_clay(TerminalSize terminal) {
    static void *memory = NULL;
    static uint32_t memory_size = 0;

    if (memory == NULL) {
        memory_size = Clay_MinMemorySize();
        memory = malloc(memory_size);
        if (memory == NULL) {
            fprintf(stderr, "failed to allocate Clay test memory\n");
            exit(1);
        }
    }

    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(memory_size, memory);
    Clay_Initialize(
        arena,
        (Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height },
        (Clay_ErrorHandler) { .errorHandlerFunction = handle_clay_error }
    );
    Clay_SetMeasureTextFunction(measure_text, NULL);
}

static int text_equals(Clay_StringSlice text, const char *expected) {
    size_t expected_length = strlen(expected);
    return text.length == (int32_t)expected_length && strncmp(text.chars, expected, expected_length) == 0;
}

static int render_contains_text(Clay_RenderCommandArray commands, const char *expected) {
    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        if (
            command->commandType == CLAY_RENDER_COMMAND_TYPE_TEXT &&
            text_equals(command->renderData.text.stringContents, expected)
        ) {
            return 1;
        }
    }

    return 0;
}

static int render_contains_text_fragment(Clay_RenderCommandArray commands, const char *expected) {
    size_t expected_length = strlen(expected);

    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        Clay_StringSlice text;

        if (command->commandType != CLAY_RENDER_COMMAND_TYPE_TEXT) {
            continue;
        }
        text = command->renderData.text.stringContents;
        if ((size_t)text.length < expected_length) {
            continue;
        }

        for (int32_t start = 0; start <= text.length - (int32_t)expected_length; start++) {
            if (strncmp(text.chars + start, expected, expected_length) == 0) {
                return 1;
            }
        }
    }

    return 0;
}

static Clay_RenderCommand *find_text_command(Clay_RenderCommandArray commands, const char *expected) {
    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        if (
            command->commandType == CLAY_RENDER_COMMAND_TYPE_TEXT &&
            text_equals(command->renderData.text.stringContents, expected)
        ) {
            return command;
        }
    }

    return NULL;
}

static int render_has_full_width_rectangle(Clay_RenderCommandArray commands, int width) {
    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        if (
            command->commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE &&
            (int)(command->boundingBox.width + 0.5f) == width
        ) {
            return 1;
        }
    }

    return 0;
}

static int render_has_command_type(Clay_RenderCommandArray commands, Clay_RenderCommandType type) {
    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        if (command->commandType == type) {
            return 1;
        }
    }

    return 0;
}

static int render_has_rectangle_near_text(Clay_RenderCommandArray commands, const char *expected, int max_distance) {
    Clay_RenderCommand *text = find_text_command(commands, expected);

    if (text == NULL) {
        return 0;
    }

    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        int distance;

        if (command->commandType != CLAY_RENDER_COMMAND_TYPE_RECTANGLE) {
            continue;
        }

        distance = (int)(command->boundingBox.y + 0.5f) - (int)(text->boundingBox.y + 0.5f);
        if (distance < 0) {
            distance = -distance;
        }
        if (distance <= max_distance && command->boundingBox.width > text->boundingBox.width) {
            return 1;
        }
    }

    return 0;
}

static Clay_RenderCommand *find_cursor_rectangle(Clay_RenderCommandArray commands) {
    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        if (
            command->commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE &&
            (int)(command->boundingBox.width + 0.5f) == 1 &&
            (int)(command->boundingBox.height + 0.5f) == 1
        ) {
            return command;
        }
    }

    return NULL;
}

static int render_text_commands_fit_within(Clay_RenderCommandArray commands, int max_width) {
    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);
        if (
            command->commandType == CLAY_RENDER_COMMAND_TYPE_TEXT &&
            command->renderData.text.stringContents.length > max_width
        ) {
            return 0;
        }
    }

    return 1;
}

SCENARIO(main_menu_contains_the_core_actions) {
    TerminalSize terminal = { .width = 160, .height = 50 };
    AppState state = { .selected_menu_index = 0 };

    GIVEN("a wide terminal");
    setup_clay(terminal);

    WHEN("the main menu is rendered");
    Clay_RenderCommandArray commands = ui_render_main_menu(&state, terminal);

    THEN("the brand and every menu action are present");
    EXPECT_TRUE(render_contains_text(commands, "tui-do"));
    EXPECT_TRUE(render_contains_text(commands, "Create a new to-do"));
    EXPECT_TRUE(render_contains_text(commands, "Load a to-do"));
    EXPECT_TRUE(render_contains_text(commands, "Quit"));
    EXPECT_TRUE(render_contains_text(commands, "q/ctrl+c"));
    EXPECT_TRUE(render_contains_text(commands, "quit"));
}

SCENARIO(main_menu_footer_lists_every_available_bind) {
    TerminalSize terminal = { .width = 160, .height = 50 };
    AppState state = { .selected_menu_index = 0 };

    GIVEN("the main menu is visible");
    setup_clay(terminal);

    WHEN("the main menu is rendered");
    Clay_RenderCommandArray commands = ui_render_main_menu(&state, terminal);

    THEN("the footer teaches every main menu bind");
    EXPECT_TRUE(render_contains_text(commands, "up/down"));
    EXPECT_TRUE(render_contains_text(commands, "move"));
    EXPECT_TRUE(render_contains_text(commands, "1-3/enter"));
    EXPECT_TRUE(render_contains_text(commands, "activate"));
    EXPECT_TRUE(render_contains_text(commands, "esc/m"));
    EXPECT_TRUE(render_contains_text(commands, "main"));
    EXPECT_TRUE(render_contains_text(commands, "q/ctrl+c"));
    EXPECT_TRUE(render_contains_text(commands, "quit"));
}

SCENARIO(main_menu_paints_the_full_terminal_width) {
    TerminalSize terminal = { .width = 160, .height = 50 };
    AppState state = { .selected_menu_index = 0 };

    GIVEN("a terminal much wider than eighty columns");
    setup_clay(terminal);

    WHEN("the main menu is rendered");
    Clay_RenderCommandArray commands = ui_render_main_menu(&state, terminal);

    THEN("at least one background rectangle spans the actual terminal width");
    EXPECT_TRUE(render_has_full_width_rectangle(commands, terminal.width));
}

SCENARIO(selected_menu_item_follows_state) {
    TerminalSize terminal = { .width = 120, .height = 40 };
    AppState state = { .selected_menu_index = 1 };

    GIVEN("the second menu item is selected");
    setup_clay(terminal);

    WHEN("the main menu is rendered");
    Clay_RenderCommandArray commands = ui_render_main_menu(&state, terminal);

    THEN("the visible selection marker is aligned with Load a to-do");
    Clay_RenderCommand *marker = find_text_command(commands, ">");
    Clay_RenderCommand *load = find_text_command(commands, "Load a to-do");
    EXPECT_TRUE(marker != NULL);
    EXPECT_TRUE(load != NULL);
    if (marker != NULL && load != NULL) {
        EXPECT_INT_EQ((int)(marker->boundingBox.y + 0.5f), (int)(load->boundingBox.y + 0.5f));
    }
}

SCENARIO(terminal_size_honors_large_environment_dimensions) {
    GIVEN("only the environment describes a large terminal");

    WHEN("terminal size candidates are resolved");
    TerminalSize terminal = terminal_resolve_size(
        (TerminalSize) { .width = 0, .height = 0 },
        (TerminalSize) { .width = 0, .height = 0 },
        (TerminalSize) { .width = 160, .height = 50 }
    );

    THEN("the resolved size is not clamped to the fallback dimensions");
    EXPECT_INT_GE(terminal.width, 160);
    EXPECT_INT_GE(terminal.height, 50);
}

SCENARIO(terminal_size_resolution_does_not_clamp_to_a_minimum) {
    GIVEN("the current terminal source reports a small size");

    WHEN("terminal size candidates are resolved");
    TerminalSize terminal = terminal_resolve_size(
        (TerminalSize) { .width = 40, .height = 12 },
        (TerminalSize) { .width = 0, .height = 0 },
        (TerminalSize) { .width = 0, .height = 0 }
    );

    THEN("the exact reported size is used");
    EXPECT_INT_EQ(terminal.width, 40);
    EXPECT_INT_EQ(terminal.height, 12);
}

SCENARIO(capture_input_renders_a_visible_cursor) {
    TerminalSize terminal = { .width = 120, .height = 40 };
    AppState state = {
        .view = APP_VIEW_CAPTURE,
        .capture_title = "AC",
        .capture_cursor = 1,
    };

    GIVEN("the capture title cursor is between two characters");
    setup_clay(terminal);

    WHEN("the capture screen is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("the input text includes a visible cursor marker");
    EXPECT_TRUE(render_contains_text_fragment(commands, "A_C"));
}

SCENARIO(capture_footer_lists_every_available_bind) {
    TerminalSize terminal = { .width = 160, .height = 50 };
    AppState state = {
        .view = APP_VIEW_CAPTURE,
        .capture_title = "Draft",
        .capture_cursor = 5,
    };

    GIVEN("the capture screen is visible");
    setup_clay(terminal);

    WHEN("the capture screen is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("the footer teaches every capture bind");
    EXPECT_TRUE(render_contains_text(commands, "type"));
    EXPECT_TRUE(render_contains_text(commands, "title"));
    EXPECT_TRUE(render_contains_text(commands, "left/right"));
    EXPECT_TRUE(render_contains_text(commands, "cursor"));
    EXPECT_TRUE(render_contains_text(commands, "backspace"));
    EXPECT_TRUE(render_contains_text(commands, "delete"));
    EXPECT_TRUE(render_contains_text(commands, "enter"));
    EXPECT_TRUE(render_contains_text(commands, "save"));
    EXPECT_TRUE(render_contains_text(commands, "esc/m"));
    EXPECT_TRUE(render_contains_text(commands, "main"));
    EXPECT_TRUE(render_contains_text(commands, "ctrl+c"));
    EXPECT_TRUE(render_contains_text(commands, "quit"));
}

SCENARIO(inbox_screen_renders_entries_and_binds) {
    TerminalSize terminal = { .width = 160, .height = 50 };
    AppState state = {
        .view = APP_VIEW_INBOX,
        .selected_inbox_index = 0,
        .inbox = {
            .entry_count = 2,
            .entries = {
                {
                    .id = "entry-2",
                    .title = "Newer note",
                    .created_at = 20,
                    .updated_at = 20,
                },
                {
                    .id = "entry-1",
                    .title = "Older note",
                    .created_at = 10,
                    .updated_at = 10,
                },
            },
        },
    };

    GIVEN("the inbox has entries");
    setup_clay(terminal);

    WHEN("the inbox is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("entries and navigation binds are visible");
    EXPECT_TRUE(render_contains_text(commands, "inbox"));
    EXPECT_TRUE(render_contains_text(commands, "Newer note"));
    EXPECT_TRUE(render_contains_text(commands, "Older note"));
    EXPECT_TRUE(render_contains_text(commands, "up/down"));
    EXPECT_TRUE(render_contains_text(commands, "move"));
    EXPECT_TRUE(render_contains_text(commands, "enter"));
    EXPECT_TRUE(render_contains_text(commands, "open"));
    EXPECT_TRUE(render_contains_text(commands, "n/1"));
    EXPECT_TRUE(render_contains_text(commands, "new"));
    EXPECT_TRUE(render_contains_text(commands, "esc/m"));
    EXPECT_TRUE(render_contains_text(commands, "main"));
}

SCENARIO(detail_screen_renders_selected_entry_shape) {
    TerminalSize terminal = { .width = 160, .height = 50 };
    AppState state = {
        .view = APP_VIEW_DETAIL,
        .selected_inbox_index = 0,
        .note_text = "# Clarify project scope\n\nWrite the actual scope here.\n- Bullet\n- [x] Done\n> Quote",
        .inbox = {
            .entry_count = 1,
            .entries = {
                {
                    .id = "entry-42",
                    .title = "Clarify project scope",
                    .created_at = 4242,
                    .updated_at = 4242,
                },
            },
        },
    };

    GIVEN("a selected entry detail is open");
    setup_clay(terminal);

    WHEN("the detail screen is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("the entry identity and markdown note shape are visible");
    EXPECT_TRUE(render_contains_text(commands, "detail"));
    EXPECT_TRUE(render_contains_text(commands, "Clarify project scope"));
    EXPECT_TRUE(render_contains_text(commands, "entry-42"));
    EXPECT_TRUE(render_contains_text(commands, "created"));
    EXPECT_TRUE(render_contains_text(commands, "4242"));
    EXPECT_TRUE(render_contains_text(commands, "markdown"));
    EXPECT_TRUE(render_contains_text(commands, "notes.md"));
    EXPECT_TRUE(render_contains_text(commands, "Preview"));
    EXPECT_TRUE(render_contains_text(commands, "Clarify project scope"));
    EXPECT_TRUE(render_contains_text(commands, "Write the actual scope here."));
    EXPECT_TRUE(render_contains_text(commands, "- Bullet"));
    EXPECT_TRUE(render_contains_text(commands, "[x] Done"));
    EXPECT_TRUE(render_contains_text(commands, "> Quote"));
    EXPECT_TRUE(render_has_rectangle_near_text(commands, "Clarify project scope", 1));
    EXPECT_TRUE(render_has_rectangle_near_text(commands, "> Quote", 1));
    EXPECT_TRUE(render_contains_text(commands, "e"));
    EXPECT_TRUE(render_contains_text(commands, "edit"));
    EXPECT_TRUE(render_contains_text(commands, "esc"));
    EXPECT_TRUE(render_contains_text(commands, "back"));
    EXPECT_TRUE(render_contains_text(commands, "m"));
    EXPECT_TRUE(render_contains_text(commands, "main"));
}

SCENARIO(note_editor_renders_text_area_save_button_and_binds) {
    TerminalSize terminal = { .width = 160, .height = 50 };
    AppState state = {
        .view = APP_VIEW_NOTE_EDITOR,
        .note_text = "# Title\n\nBody",
        .note_cursor = 10,
    };

    GIVEN("the note editor is open");
    setup_clay(terminal);

    WHEN("the note editor is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("the text area, cursor, save button, and binds are visible");
    EXPECT_TRUE(render_contains_text(commands, "editor"));
    EXPECT_TRUE(render_contains_text(commands, "Markdown editor"));
    EXPECT_TRUE(render_contains_text(commands, "Ln 3, Col 2"));
    EXPECT_TRUE(render_contains_text(commands, "01"));
    EXPECT_TRUE(render_contains_text(commands, "03"));
    EXPECT_TRUE(render_contains_text(commands, "# Title"));
    EXPECT_TRUE(render_contains_text(commands, "Body"));
    EXPECT_TRUE(!render_contains_text_fragment(commands, "B_ody"));
    EXPECT_TRUE(render_has_rectangle_near_text(commands, "Body", 1));
    Clay_RenderCommand *body = find_text_command(commands, "Body");
    Clay_RenderCommand *cursor = find_cursor_rectangle(commands);
    EXPECT_TRUE(body != NULL);
    EXPECT_TRUE(cursor != NULL);
    if (body != NULL && cursor != NULL) {
        THEN("the cursor is painted at the insertion point instead of covering the previous character");
        EXPECT_INT_EQ((int)(cursor->boundingBox.x + 0.5f), (int)(body->boundingBox.x + 0.5f) + 1);
        EXPECT_INT_EQ((int)(cursor->boundingBox.y + 0.5f), (int)(body->boundingBox.y + 0.5f));
    }
    EXPECT_TRUE(render_contains_text(commands, "Save"));
    EXPECT_TRUE(render_contains_text(commands, "arrows"));
    EXPECT_TRUE(render_contains_text(commands, "cursor"));
    EXPECT_TRUE(render_contains_text(commands, "shift+enter"));
    EXPECT_TRUE(render_contains_text(commands, "newline"));
    EXPECT_TRUE(render_contains_text(commands, "ctrl+s"));
    EXPECT_TRUE(render_contains_text(commands, "save"));
    EXPECT_TRUE(render_contains_text(commands, "ctrl+c"));
    EXPECT_TRUE(!render_contains_text(commands, "q/ctrl+c"));
}

SCENARIO(note_editor_clips_to_the_visible_terminal_area) {
    TerminalSize terminal = { .width = 80, .height = 18 };
    AppState state = {
        .view = APP_VIEW_NOTE_EDITOR,
        .note_text = "Line 01\nLine 02\nLine 03\nLine 04\nLine 05\nLine 06\nLine 07\nLine 08\nLine 09\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\nLine 18\nLine 19\nLine 20",
        .note_cursor = 0,
    };

    GIVEN("a note that is taller than the editor viewport");
    setup_clay(terminal);

    WHEN("the note editor is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("the editor marks the text area as clipped and keeps overflow out of the footer");
    EXPECT_TRUE(render_has_command_type(commands, CLAY_RENDER_COMMAND_TYPE_SCISSOR_START));
    EXPECT_TRUE(render_contains_text(commands, "Line 01"));
    EXPECT_TRUE(!render_contains_text(commands, "Line 20"));
}

SCENARIO(note_editor_scrolls_to_keep_the_cursor_visible) {
    TerminalSize terminal = { .width = 80, .height = 18 };
    const char *note = "Line 01\nLine 02\nLine 03\nLine 04\nLine 05\nLine 06\nLine 07\nLine 08\nLine 09\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\nLine 18\nLine 19\nLine 20";
    AppState state = {
        .view = APP_VIEW_NOTE_EDITOR,
        .note_cursor = (int)strlen("Line 01\nLine 02\nLine 03\nLine 04\nLine 05\nLine 06\nLine 07\nLine 08\nLine 09\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\nLine 18\nLine 19\nLine 20"),
    };
    snprintf(state.note_text, sizeof(state.note_text), "%s", note);

    GIVEN("the cursor is near the bottom of a long note");
    setup_clay(terminal);

    WHEN("the note editor is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("the visible editor window follows the cursor");
    EXPECT_TRUE(render_contains_text(commands, "Line 20"));
    EXPECT_TRUE(!render_contains_text(commands, "Line 01"));
    EXPECT_TRUE(find_cursor_rectangle(commands) != NULL);
}

SCENARIO(note_editor_horizontally_scrolls_long_lines_to_the_cursor) {
    TerminalSize terminal = { .width = 40, .height = 24 };
    AppState state = {
        .view = APP_VIEW_NOTE_EDITOR,
        .note_text = "abcdefghijklmnopqrstuvwxyz",
        .note_cursor = 26,
    };

    GIVEN("the cursor is beyond the right edge of a long editor line");
    setup_clay(terminal);

    WHEN("the note editor is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("the visible text follows the cursor instead of overflowing the editor");
    EXPECT_TRUE(render_contains_text_fragment(commands, "xyz"));
    EXPECT_TRUE(!render_contains_text_fragment(commands, "abcdefghijkl"));
    EXPECT_TRUE(render_text_commands_fit_within(commands, terminal.width - 8));
    EXPECT_TRUE(find_cursor_rectangle(commands) != NULL);
}

SCENARIO(markdown_preview_wraps_long_paragraphs_inside_the_terminal) {
    TerminalSize terminal = { .width = 64, .height = 28 };
    AppState state = {
        .view = APP_VIEW_DETAIL,
        .selected_inbox_index = 0,
        .note_text = "This is a very long markdown paragraph that should wrap into smaller terminal-friendly visual lines instead of spilling across the whole screen.",
        .inbox = {
            .entry_count = 1,
            .entries = {
                {
                    .id = "entry-wrap",
                    .title = "Wrap preview",
                    .created_at = 1,
                    .updated_at = 1,
                },
            },
        },
    };

    GIVEN("a markdown note has a paragraph wider than the preview pane");
    setup_clay(terminal);

    WHEN("the detail screen is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("the preview emits wrapped text commands rather than one overflowing line");
    EXPECT_TRUE(render_contains_text_fragment(commands, "terminal-friendly"));
    EXPECT_TRUE(render_text_commands_fit_within(commands, terminal.width - 8));
}

SCENARIO(markdown_preview_scrolls_past_earlier_visual_lines) {
    TerminalSize terminal = { .width = 64, .height = 28 };
    AppState state = {
        .view = APP_VIEW_DETAIL,
        .selected_inbox_index = 0,
        .detail_scroll_line = 2,
        .note_text = "First visual line should disappear when scrolling through a long markdown paragraph that keeps going for several wrapped rows inside the preview pane.",
        .inbox = {
            .entry_count = 1,
            .entries = {
                {
                    .id = "entry-scroll",
                    .title = "Scroll preview",
                    .created_at = 1,
                    .updated_at = 1,
                },
            },
        },
    };

    GIVEN("the markdown preview has been scrolled down");
    setup_clay(terminal);

    WHEN("the detail screen is rendered");
    Clay_RenderCommandArray commands = ui_render_app(&state, terminal);

    THEN("earlier visual lines are skipped and later lines remain visible");
    EXPECT_TRUE(!render_contains_text_fragment(commands, "First visual"));
    EXPECT_TRUE(render_contains_text_fragment(commands, "preview pane"));
}

int main(void) {
    RUN_SCENARIO(main_menu_contains_the_core_actions);
    RUN_SCENARIO(main_menu_footer_lists_every_available_bind);
    RUN_SCENARIO(main_menu_paints_the_full_terminal_width);
    RUN_SCENARIO(selected_menu_item_follows_state);
    RUN_SCENARIO(terminal_size_honors_large_environment_dimensions);
    RUN_SCENARIO(terminal_size_resolution_does_not_clamp_to_a_minimum);
    RUN_SCENARIO(capture_input_renders_a_visible_cursor);
    RUN_SCENARIO(capture_footer_lists_every_available_bind);
    RUN_SCENARIO(inbox_screen_renders_entries_and_binds);
    RUN_SCENARIO(detail_screen_renders_selected_entry_shape);
    RUN_SCENARIO(note_editor_renders_text_area_save_button_and_binds);
    RUN_SCENARIO(note_editor_clips_to_the_visible_terminal_area);
    RUN_SCENARIO(note_editor_scrolls_to_keep_the_cursor_visible);
    RUN_SCENARIO(note_editor_horizontally_scrolls_long_lines_to_the_cursor);
    RUN_SCENARIO(markdown_preview_wraps_long_paragraphs_inside_the_terminal);
    RUN_SCENARIO(markdown_preview_scrolls_past_earlier_visual_lines);

    return finish_tests();
}
