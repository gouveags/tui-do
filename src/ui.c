#include "ui.h"

#include "markdown.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const Clay_Color COLOR_BACKGROUND = {18, 22, 28, 255};
static const Clay_Color COLOR_SURFACE = {28, 35, 44, 255};
static const Clay_Color COLOR_SURFACE_RAISED = {34, 42, 52, 255};
static const Clay_Color COLOR_SURFACE_ACTIVE = {33, 55, 55, 255};
static const Clay_Color COLOR_TEXT = {234, 239, 237, 255};
static const Clay_Color COLOR_MUTED = {143, 153, 156, 255};
static const Clay_Color COLOR_ACCENT = {70, 221, 181, 255};
static const Clay_Color COLOR_SELECTED = {219, 255, 242, 255};
static const Clay_Color COLOR_CURSOR = {219, 255, 242, 255};

typedef struct UiBind {
    const char *keys;
    const char *label;
} UiBind;

static Clay_String clay_string(const char *text) {
    return (Clay_String) {
        .length = (int32_t)strlen(text),
        .chars = text,
    };
}

static char *ui_scratch_text(void) {
    static char buffers[128][256];
    static int index = 0;

    index = (index + 1) % 128;
    buffers[index][0] = '\0';
    return buffers[index];
}

static void draw_text(const char *text, Clay_Color color) {
    CLAY_TEXT(
        clay_string(text),
        CLAY_TEXT_CONFIG({
            .textColor = color,
            .fontSize = 1,
        })
    );
}

static void draw_wrapped_text_scrolled(const char *text, Clay_Color color, int max_width, int *skip_lines) {
    size_t length = strlen(text);
    size_t offset = 0;

    if (max_width < 8) {
        max_width = 8;
    }

    while (offset < length) {
        char *line = ui_scratch_text();
        size_t remaining = length - offset;
        size_t take = remaining < (size_t)max_width ? remaining : (size_t)max_width;
        size_t break_at = take;

        if (take == (size_t)max_width && offset + take < length) {
            for (size_t i = take; i > 0; i--) {
                if (text[offset + i] == ' ') {
                    break_at = i;
                    break;
                }
            }
        }
        if (break_at == 0) {
            break_at = take;
        }

        if (*skip_lines > 0) {
            (*skip_lines)--;
        } else {
            snprintf(line, 256, "%.*s", (int)break_at, text + offset);
            draw_text(line, color);
        }

        offset += break_at;
        while (text[offset] == ' ') {
            offset++;
        }
    }

    if (length == 0 && *skip_lines > 0) {
        (*skip_lines)--;
    } else if (length == 0) {
        draw_text(" ", color);
    }
}

static void draw_pill_text(const char *text, Clay_Color foreground, Clay_Color background) {
    CLAY(CLAY_IDI("Pill", (int)(uintptr_t)text), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIT(0),
                .height = CLAY_SIZING_FIXED(3),
            },
            .padding = {
                .left = 2,
                .right = 2,
                .top = 1,
            },
        },
        .backgroundColor = background,
    }) {
        draw_text(text, foreground);
    }
}

static void build_cursor_text(const AppState *state, char *out, size_t out_size) {
    size_t length = strlen(state->capture_title);
    size_t cursor = state->capture_cursor < 0 ? 0 : (size_t)state->capture_cursor;

    if (cursor > length) {
        cursor = length;
    }
    if (out_size == 0) {
        return;
    }

    if (length + 2 > out_size) {
        length = out_size - 2;
        if (cursor > length) {
            cursor = length;
        }
    }

    memcpy(out, state->capture_title, cursor);
    out[cursor] = '_';
    memcpy(out + cursor + 1, state->capture_title + cursor, length - cursor);
    out[length + 1] = '\0';
}

static void describe_cursor_position(const char *text, int cursor, int *line_out, int *column_out) {
    int line = 1;
    int column = 1;
    int length = (int)strlen(text);

    if (cursor < 0) {
        cursor = 0;
    }
    if (cursor > length) {
        cursor = length;
    }

    for (int i = 0; i < cursor; i++) {
        if (text[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }

    *line_out = line;
    *column_out = column;
}

static void copy_note_line(const char *text, int line_index, char *out, size_t out_size) {
    int current_line = 0;
    size_t offset = 0;

    if (out_size == 0) {
        return;
    }

    out[0] = '\0';
    for (size_t i = 0;; i++) {
        char value = text[i];

        if (current_line == line_index && value != '\n' && value != '\0') {
            if (offset + 1 < out_size) {
                out[offset++] = value;
            }
        }
        if (value == '\n' || value == '\0') {
            if (current_line == line_index) {
                out[offset] = '\0';
                return;
            }
            current_line++;
        }
        if (value == '\0') {
            out[offset] = '\0';
            return;
        }
    }
}

static void copy_note_line_window(const char *text, int line_index, int first_column, int max_columns, char *out, size_t out_size) {
    char full_line[256];
    size_t length;

    copy_note_line(text, line_index, full_line, sizeof(full_line));
    length = strlen(full_line);
    if (first_column < 0) {
        first_column = 0;
    }
    if (max_columns < 1) {
        max_columns = 1;
    }
    if ((size_t)first_column > length) {
        first_column = (int)length;
    }
    if ((size_t)max_columns >= out_size) {
        max_columns = (int)out_size - 1;
    }

    snprintf(out, out_size, "%.*s", max_columns, full_line + first_column);
}

static int count_note_lines(const char *text) {
    int lines = 1;

    for (size_t i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            lines++;
        }
    }

    return lines;
}

static const char *prefixed_markdown_text(const char *prefix, const char *value) {
    char *text = ui_scratch_text();
    size_t prefix_length = strlen(prefix);
    size_t text_size = 128;
    size_t value_size = text_size - prefix_length - 1;

    snprintf(text, text_size, "%s%.*s", prefix, (int)value_size, value);
    return text;
}

static const char *markdown_display_text(const MarkdownBlock *block) {
    switch (block->type) {
        case MARKDOWN_BLOCK_BLANK:
            return " ";
        case MARKDOWN_BLOCK_UNORDERED_LIST_ITEM:
            return prefixed_markdown_text("- ", block->text);
        case MARKDOWN_BLOCK_ORDERED_LIST_ITEM:
            return prefixed_markdown_text("1. ", block->text);
        case MARKDOWN_BLOCK_TASK_OPEN:
            return prefixed_markdown_text("[ ] ", block->text);
        case MARKDOWN_BLOCK_TASK_DONE:
            return prefixed_markdown_text("[x] ", block->text);
        case MARKDOWN_BLOCK_QUOTE:
            return prefixed_markdown_text("> ", block->text);
        case MARKDOWN_BLOCK_THEMATIC_BREAK:
            return "----------------";
        default:
            return block->text;
    }
}

static Clay_Color markdown_color(MarkdownBlockType type) {
    switch (type) {
        case MARKDOWN_BLOCK_HEADING_1:
            return COLOR_SELECTED;
        case MARKDOWN_BLOCK_HEADING_2:
        case MARKDOWN_BLOCK_HEADING_3:
        case MARKDOWN_BLOCK_THEMATIC_BREAK:
            return COLOR_ACCENT;
        case MARKDOWN_BLOCK_QUOTE:
        case MARKDOWN_BLOCK_CODE:
        case MARKDOWN_BLOCK_BLANK:
            return COLOR_MUTED;
        default:
            return COLOR_TEXT;
    }
}

static void draw_markdown_preview(const char *markdown, int max_width, int scroll_line) {
    MarkdownDocument document = markdown_parse(markdown);
    int skip_lines = scroll_line;

    for (size_t i = 0; i < document.block_count && i < 16; i++) {
        MarkdownBlock *block = &document.blocks[i];
        Clay_Color background = block->type == MARKDOWN_BLOCK_QUOTE || block->type == MARKDOWN_BLOCK_CODE
            ? COLOR_SURFACE_RAISED
            : block->type == MARKDOWN_BLOCK_HEADING_1
                ? COLOR_SURFACE_ACTIVE
                : COLOR_BACKGROUND;

        CLAY(CLAY_IDI("MarkdownBlock", (int)i), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIT(0),
                },
                .padding = {
                    .left = 2,
                    .right = 2,
                    .top = block->type == MARKDOWN_BLOCK_BLANK ? 0 : 1,
                    .bottom = block->type == MARKDOWN_BLOCK_BLANK ? 0 : 1,
                },
            },
            .backgroundColor = background,
        }) {
            draw_wrapped_text_scrolled(markdown_display_text(block), markdown_color(block->type), max_width, &skip_lines);
        }
    }
}

static void draw_top_bar(const char *label) {
    CLAY(CLAY_ID("TopBar"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIXED(3),
            },
            .padding = {
                .left = 3,
                .right = 3,
                .top = 1,
            },
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
        .backgroundColor = COLOR_SURFACE,
    }) {
        draw_text("tui-do", COLOR_ACCENT);
        CLAY(CLAY_ID("TopBarSpacer"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIXED(1),
                },
            },
        }) {}
        draw_text(label, COLOR_MUTED);
    }
}

static void draw_hero(bool compact) {
    CLAY(CLAY_ID("Hero"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIT(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = compact ? 0 : 1,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
            },
        },
    }) {
        if (compact) {
            draw_text("tui-do", COLOR_SELECTED);
        } else {
            draw_text("tui-do", COLOR_SELECTED);
            draw_text("Small plans, clearly kept.", COLOR_MUTED);
        }
    }
}

static void draw_menu_item(int index, const char *number, const char *title, const char *description, bool selected, bool compact) {
    CLAY(CLAY_IDI("MenuItem", index), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIXED(compact ? 3 : 4),
            },
            .padding = {
                .left = 2,
                .right = 2,
                .top = compact ? 0 : 1,
            },
            .childGap = 2,
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
        .backgroundColor = selected ? COLOR_SURFACE_ACTIVE : COLOR_SURFACE,
    }) {
        draw_text(selected ? ">" : " ", selected ? COLOR_ACCENT : COLOR_MUTED);
        draw_text(number, selected ? COLOR_ACCENT : COLOR_MUTED);
        CLAY(CLAY_IDI("MenuCopy", index), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIT(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        }) {
            draw_text(title, selected ? COLOR_SELECTED : COLOR_TEXT);
            draw_text(description, COLOR_MUTED);
        }
    }
}

static void draw_menu(AppState *state, bool compact) {
    CLAY(CLAY_ID("MenuPanel"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_PERCENT(compact ? 0.92f : 0.48f),
                .height = CLAY_SIZING_FIT(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = {
                .left = compact ? 0 : 1,
                .right = compact ? 0 : 1,
                .top = compact ? 0 : 1,
                .bottom = compact ? 0 : 1,
            },
            .childGap = 1,
        },
        .backgroundColor = {22, 28, 36, 255},
    }) {
        draw_menu_item(0, "01", "Create a new to-do", "Start with an empty list", state->selected_menu_index == 0, compact);
        draw_menu_item(1, "02", "Load a to-do", "Continue an existing list", state->selected_menu_index == 1, compact);
        draw_menu_item(2, "03", "Quit", "Return to the terminal", state->selected_menu_index == 2, compact);
    }
}

static void draw_footer(const UiBind *binds, size_t bind_count) {
    CLAY(CLAY_ID("Footer"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIXED(2),
            },
            .padding = {
                .left = 3,
                .right = 3,
                .top = 0,
            },
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
            .childGap = 3,
        },
        .backgroundColor = COLOR_SURFACE,
    }) {
        for (size_t i = 0; i < bind_count; i++) {
            draw_text(binds[i].keys, COLOR_ACCENT);
            draw_text(binds[i].label, COLOR_MUTED);
        }
    }
}

static void draw_main_footer(void) {
    static const UiBind binds[] = {
        { "up/down", "move" },
        { "1-3/enter", "activate" },
        { "esc/m", "main" },
        { "q/ctrl+c", "quit" },
    };

    draw_footer(binds, sizeof(binds) / sizeof(binds[0]));
}

Clay_RenderCommandArray ui_render_main_menu(AppState *state, TerminalSize terminal) {
    bool compact = terminal.width < 96 || terminal.height <= 28;

    Clay_SetLayoutDimensions((Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("Root"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = {
                .left = 2,
                .right = 2,
                .top = 1,
                .bottom = 1,
            },
            .childGap = compact ? 1 : 2,
        },
        .backgroundColor = COLOR_BACKGROUND,
    }) {
        draw_top_bar("main menu");
        CLAY(CLAY_ID("Body"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childGap = compact ? 1 : 4,
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                    .y = CLAY_ALIGN_Y_CENTER,
                },
            },
        }) {
            draw_hero(compact);
            draw_menu(state, compact);
        }
        draw_main_footer();
    }

    return Clay_EndLayout(0.0f);
}

static void draw_capture_footer(void) {
    static const UiBind binds[] = {
        { "type", "title" },
        { "left/right", "cursor" },
        { "backspace", "delete" },
        { "enter", "save" },
        { "esc/m", "main" },
        { "ctrl+c", "quit" },
    };

    draw_footer(binds, sizeof(binds) / sizeof(binds[0]));
}

static void draw_inbox_footer(void) {
    static const UiBind binds[] = {
        { "up/down", "move" },
        { "enter", "open" },
        { "n/1", "new" },
        { "esc/m", "main" },
        { "q/ctrl+c", "quit" },
    };

    draw_footer(binds, sizeof(binds) / sizeof(binds[0]));
}

static void draw_detail_footer(void) {
    static const UiBind binds[] = {
        { "e", "edit" },
        { "pg/wheel", "scroll" },
        { "esc", "back" },
        { "m", "main" },
        { "q/ctrl+c", "quit" },
    };

    draw_footer(binds, sizeof(binds) / sizeof(binds[0]));
}

static void draw_note_editor_footer(void) {
    static const UiBind binds[] = {
        { "type", "text" },
        { "arrows", "cursor" },
        { "pg/wheel", "scroll" },
        { "shift+enter", "newline" },
        { "ctrl+s", "save" },
        { "esc", "back" },
        { "ctrl+c", "quit" },
    };

    draw_footer(binds, sizeof(binds) / sizeof(binds[0]));
}

static void draw_inbox_entry(int index, const TodoIndexEntry *entry, bool selected) {
    char *meta = ui_scratch_text();

    snprintf(meta, 128, "created %lld  items %zu/%zu", entry->created_at, entry->done_count, entry->item_count);
    CLAY(CLAY_IDI("InboxEntry", index), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_FIXED(4),
            },
            .padding = {
                .left = 2,
                .right = 2,
                .top = 1,
            },
            .childGap = 2,
            .childAlignment = {
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
        .backgroundColor = selected ? COLOR_SURFACE_ACTIVE : COLOR_SURFACE,
    }) {
        draw_text(selected ? ">" : " ", selected ? COLOR_ACCENT : COLOR_MUTED);
        CLAY(CLAY_IDI("InboxEntryCopy", index), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIT(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        }) {
            draw_text(entry->title, selected ? COLOR_SELECTED : COLOR_TEXT);
            draw_text(meta, COLOR_MUTED);
        }
    }
}

static Clay_RenderCommandArray ui_render_inbox(AppState *state, TerminalSize terminal) {
    Clay_SetLayoutDimensions((Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("InboxRoot"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = {
                .left = 2,
                .right = 2,
                .top = 1,
                .bottom = 1,
            },
            .childGap = 2,
        },
        .backgroundColor = COLOR_BACKGROUND,
    }) {
        draw_top_bar("inbox");
        CLAY(CLAY_ID("InboxBody"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childGap = 1,
            },
        }) {
            if (state->inbox.entry_count == 0) {
                draw_text("No entries yet.", COLOR_MUTED);
            }
            for (size_t i = 0; i < state->inbox.entry_count && i < 12; i++) {
                draw_inbox_entry((int)i, &state->inbox.entries[i], state->selected_inbox_index == (int)i);
            }
        }
        draw_inbox_footer();
    }

    return Clay_EndLayout(0.0f);
}

static Clay_RenderCommandArray ui_render_capture(AppState *state, TerminalSize terminal) {
    Clay_SetLayoutDimensions((Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("CaptureRoot"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = {
                .left = 2,
                .right = 2,
                .top = 1,
                .bottom = 1,
            },
            .childGap = 2,
        },
        .backgroundColor = COLOR_BACKGROUND,
    }) {
        draw_top_bar("capture");
        CLAY(CLAY_ID("CaptureBody"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childGap = 2,
                .childAlignment = {
                    .x = CLAY_ALIGN_X_CENTER,
                    .y = CLAY_ALIGN_Y_CENTER,
                },
            },
        }) {
            draw_text("Capture a new notebook entry", COLOR_SELECTED);
            draw_text("Write the thing before it evaporates.", COLOR_MUTED);
            CLAY(CLAY_ID("CaptureInput"), {
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_PERCENT(0.72f),
                        .height = CLAY_SIZING_FIXED(5),
                    },
                    .padding = {
                        .left = 2,
                        .right = 2,
                        .top = 1,
                    },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .childGap = 1,
                },
                .backgroundColor = COLOR_SURFACE_ACTIVE,
            }) {
                char input_text[sizeof(state->capture_title) + 2];
                build_cursor_text(state, input_text, sizeof(input_text));
                draw_text("Title", COLOR_ACCENT);
                draw_text(input_text, COLOR_TEXT);
            }
        }
        draw_capture_footer();
    }

    return Clay_EndLayout(0.0f);
}

static Clay_RenderCommandArray ui_render_detail(AppState *state, TerminalSize terminal) {
    TodoIndexEntry *entry = NULL;
    char *created = ui_scratch_text();

    if (
        state->selected_inbox_index >= 0 &&
        state->selected_inbox_index < (int)state->inbox.entry_count
    ) {
        entry = &state->inbox.entries[state->selected_inbox_index];
    }

    Clay_SetLayoutDimensions((Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("DetailRoot"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = {
                .left = 2,
                .right = 2,
                .top = 1,
                .bottom = 1,
            },
            .childGap = 2,
        },
        .backgroundColor = COLOR_BACKGROUND,
    }) {
        draw_top_bar("detail");
        CLAY(CLAY_ID("DetailBody"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childGap = 1,
                .padding = {
                    .left = 3,
                    .right = 3,
                    .top = 2,
                },
            },
        }) {
            if (entry == NULL) {
                draw_text("No entry selected.", COLOR_MUTED);
            } else {
                snprintf(created, 128, "%lld", entry->created_at);
                CLAY(CLAY_ID("DetailMeta"), {
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_FIT(0),
                        },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .padding = {
                            .left = 2,
                            .right = 2,
                            .top = 1,
                            .bottom = 1,
                        },
                        .childGap = 1,
                    },
                    .backgroundColor = COLOR_SURFACE,
                }) {
                    draw_text(entry->title, COLOR_SELECTED);
                    draw_text(entry->id, COLOR_MUTED);
                    CLAY(CLAY_ID("DetailMetaRow"), {
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIT(0),
                            },
                            .childGap = 2,
                        },
                    }) {
                        draw_text("created", COLOR_ACCENT);
                        draw_text(created, COLOR_TEXT);
                        draw_text("markdown", COLOR_ACCENT);
                        draw_text("notes.md", COLOR_TEXT);
                    }
                }
                CLAY(CLAY_ID("MarkdownPreview"), {
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_GROW(0),
                        },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .padding = {
                            .left = 2,
                            .right = 2,
                            .top = 1,
                            .bottom = 1,
                        },
                        .childGap = 1,
                    },
                    .backgroundColor = COLOR_BACKGROUND,
                    .clip = {
                        .horizontal = true,
                        .vertical = true,
                    },
                }) {
                    draw_pill_text("Preview", COLOR_ACCENT, COLOR_SURFACE_RAISED);
                    draw_markdown_preview(state->note_text, terminal.width - 12, state->detail_scroll_line);
                }
            }
        }
        draw_detail_footer();
    }

    return Clay_EndLayout(0.0f);
}

static Clay_RenderCommandArray ui_render_note_editor(AppState *state, TerminalSize terminal) {
    int cursor_line;
    int cursor_column;
    int total_lines = count_note_lines(state->note_text);
    int visible_lines = terminal.height - 16;
    int first_visible_line = 0;
    int max_editor_columns = terminal.width - 16;
    int first_visible_column = 0;
    char *status = ui_scratch_text();

    if (visible_lines < 1) {
        visible_lines = 1;
    }
    if (visible_lines > total_lines) {
        visible_lines = total_lines;
    }
    describe_cursor_position(state->note_text, state->note_cursor, &cursor_line, &cursor_column);
    first_visible_line = state->note_scroll_line;
    if (first_visible_line < 0) {
        first_visible_line = 0;
    }
    if (cursor_line <= first_visible_line) {
        first_visible_line = cursor_line - 1;
    }
    if (cursor_line > first_visible_line + visible_lines) {
        first_visible_line = cursor_line - visible_lines;
    }
    if (max_editor_columns < 8) {
        max_editor_columns = 8;
    }
    if (cursor_column > max_editor_columns) {
        first_visible_column = cursor_column - max_editor_columns;
    }
    snprintf(status, 256, "Ln %d, Col %d", cursor_line, cursor_column);

    Clay_SetLayoutDimensions((Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("NoteEditorRoot"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = {
                .left = 2,
                .right = 2,
                .top = 1,
                .bottom = 1,
            },
            .childGap = 2,
        },
        .backgroundColor = COLOR_BACKGROUND,
    }) {
        draw_top_bar("editor");
        CLAY(CLAY_ID("NoteEditorBody"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .padding = {
                    .left = 3,
                    .right = 3,
                    .top = 2,
                },
                .childGap = 2,
            },
        }) {
            CLAY(CLAY_ID("NoteEditorHeader"), {
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_FIXED(3),
                    },
                    .padding = {
                        .left = 2,
                        .right = 2,
                        .top = 1,
                    },
                    .childGap = 2,
                    .childAlignment = {
                        .y = CLAY_ALIGN_Y_CENTER,
                    },
                },
                .backgroundColor = COLOR_SURFACE,
            }) {
                draw_text("Markdown editor", COLOR_SELECTED);
                CLAY(CLAY_ID("NoteEditorHeaderSpacer"), {
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(0),
                            .height = CLAY_SIZING_FIXED(1),
                        },
                    },
                }) {}
                draw_text(status, COLOR_MUTED);
            }
            CLAY(CLAY_ID("NoteEditorTextArea"), {
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_GROW(0),
                    },
                    .padding = {
                        .left = 2,
                        .right = 2,
                        .top = 1,
                        .bottom = 1,
                    },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = COLOR_SURFACE_ACTIVE,
                .clip = {
                    .horizontal = true,
                    .vertical = true,
                },
            }) {
                for (int i = 0; i < visible_lines; i++) {
                    char *number = ui_scratch_text();
                    char *line = ui_scratch_text();
                    int line_index = first_visible_line + i;
                    bool active = line_index + 1 == cursor_line;

                    snprintf(number, 256, "%02d", line_index + 1);
                    copy_note_line_window(state->note_text, line_index, first_visible_column, max_editor_columns, line, 256);

                    CLAY(CLAY_IDI("NoteEditorLine", i), {
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_FIXED(1),
                            },
                            .padding = {
                                .left = 1,
                                .right = 1,
                            },
                            .childGap = 2,
                            .childAlignment = {
                                .y = CLAY_ALIGN_Y_CENTER,
                            },
                        },
                        .backgroundColor = active ? COLOR_SURFACE_RAISED : COLOR_SURFACE_ACTIVE,
                    }) {
                        draw_text(number, active ? COLOR_ACCENT : COLOR_MUTED);
                        draw_text(line[0] == '\0' ? " " : line, COLOR_TEXT);
                    }
                }
                if (cursor_line >= first_visible_line + 1 && cursor_line <= first_visible_line + visible_lines) {
                    int cursor_visible_line = cursor_line - first_visible_line - 1;

                    CLAY(CLAY_ID("NoteEditorCursor"), {
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_FIXED(1),
                                .height = CLAY_SIZING_FIXED(1),
                            },
                        },
                        .backgroundColor = COLOR_CURSOR,
                        .floating = {
                            .attachTo = CLAY_ATTACH_TO_PARENT,
                            .attachPoints = {
                                .element = CLAY_ATTACH_POINT_LEFT_TOP,
                                .parent = CLAY_ATTACH_POINT_LEFT_TOP,
                            },
                            .offset = {
                                .x = (float)(7 + cursor_column - first_visible_column - 1),
                                .y = (float)(1 + cursor_visible_line),
                            },
                            .clipTo = CLAY_CLIP_TO_ATTACHED_PARENT,
                        },
                    }) {}
                }
            }
            CLAY(CLAY_ID("NoteEditorSaveButton"), {
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIT(0),
                        .height = CLAY_SIZING_FIXED(3),
                    },
                    .padding = {
                        .left = 2,
                        .right = 2,
                        .top = 1,
                    },
                },
                .backgroundColor = COLOR_SURFACE,
            }) {
                draw_text("Save", COLOR_ACCENT);
            }
        }
        draw_note_editor_footer();
    }

    return Clay_EndLayout(0.0f);
}

Clay_RenderCommandArray ui_render_app(AppState *state, TerminalSize terminal) {
    if (state->view == APP_VIEW_CAPTURE) {
        return ui_render_capture(state, terminal);
    }
    if (state->view == APP_VIEW_INBOX) {
        return ui_render_inbox(state, terminal);
    }
    if (state->view == APP_VIEW_DETAIL) {
        return ui_render_detail(state, terminal);
    }
    if (state->view == APP_VIEW_NOTE_EDITOR) {
        return ui_render_note_editor(state, terminal);
    }

    return ui_render_main_menu(state, terminal);
}
