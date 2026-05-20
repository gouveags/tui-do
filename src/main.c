#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define CLAY_IMPLEMENTATION
#include "../vendor/clay/clay.h"

#define FALLBACK_TERMINAL_WIDTH 80
#define FALLBACK_TERMINAL_HEIGHT 24

typedef struct TerminalSize {
    int width;
    int height;
} TerminalSize;

static void handle_clay_error(Clay_ErrorData error_data) {
    fprintf(stderr, "Clay error: %.*s\n", error_data.errorText.length, error_data.errorText.chars);
}

static Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data) {
    (void)config;
    (void)user_data;

    return (Clay_Dimensions) {
        .width = (float)text.length,
        .height = 1.0f,
    };
}

static TerminalSize get_terminal_size(void) {
    struct winsize size;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0 && size.ws_row > 0) {
        return (TerminalSize) {
            .width = size.ws_col,
            .height = size.ws_row,
        };
    }

    return (TerminalSize) {
        .width = FALLBACK_TERMINAL_WIDTH,
        .height = FALLBACK_TERMINAL_HEIGHT,
    };
}

static char *screen_cell(char *screen, TerminalSize terminal, int x, int y) {
    return &screen[(y * (terminal.width + 1)) + x];
}

static void leave_full_screen(void) {
    printf("\033[?25h\033[?1049l");
    fflush(stdout);
}

static void draw_text(char *screen, TerminalSize terminal, Clay_RenderCommand *command) {
    Clay_TextRenderData text = command->renderData.text;
    int x = (int)(command->boundingBox.x + 0.5f);
    int y = (int)(command->boundingBox.y + 0.5f);

    if (y < 0 || y >= terminal.height) {
        return;
    }

    for (int32_t i = 0; i < text.stringContents.length; i++) {
        int column = x + i;
        if (column >= 0 && column < terminal.width) {
            *screen_cell(screen, terminal, column, y) = text.stringContents.chars[i];
        }
    }
}

static void render_to_terminal(Clay_RenderCommandArray commands, TerminalSize terminal) {
    size_t row_length = (size_t)terminal.width + 1;
    size_t screen_size = row_length * (size_t)terminal.height;
    char *screen = malloc(screen_size);

    if (screen == NULL) {
        fprintf(stderr, "Failed to allocate terminal buffer.\n");
        return;
    }

    for (int y = 0; y < terminal.height; y++) {
        char *row = screen_cell(screen, terminal, 0, y);
        memset(row, ' ', terminal.width);
        row[terminal.width] = '\0';
    }

    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);

        if (command->commandType == CLAY_RENDER_COMMAND_TYPE_TEXT) {
            draw_text(screen, terminal, command);
        }
    }

    printf("\033[?1049h\033[?25l\033[2J\033[H");
    for (int y = 0; y < terminal.height; y++) {
        puts(screen_cell(screen, terminal, 0, y));
    }
    fflush(stdout);

    free(screen);
}

int main(void) {
    atexit(leave_full_screen);

    TerminalSize terminal = get_terminal_size();
    uint32_t clay_memory_size = Clay_MinMemorySize();
    void *clay_memory = malloc(clay_memory_size);
    if (clay_memory == NULL) {
        fprintf(stderr, "Failed to allocate Clay arena.\n");
        return 1;
    }

    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(clay_memory_size, clay_memory);
    Clay_Initialize(
        arena,
        (Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height },
        (Clay_ErrorHandler) { .errorHandlerFunction = handle_clay_error }
    );
    Clay_SetMeasureTextFunction(measure_text, NULL);

    Clay_SetLayoutDimensions((Clay_Dimensions) { .width = (float)terminal.width, .height = (float)terminal.height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("Root"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 1,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER,
            },
        },
    }) {
        CLAY_TEXT(
            CLAY_STRING("Hello from Clay"),
            CLAY_TEXT_CONFIG({
                .textColor = {255, 255, 255, 255},
                .fontSize = 1,
            })
        );
        CLAY_TEXT(
            CLAY_STRING("Press Enter to exit"),
            CLAY_TEXT_CONFIG({
                .textColor = {160, 160, 160, 255},
                .fontSize = 1,
            })
        );
    }

    Clay_RenderCommandArray commands = Clay_EndLayout(0.0f);
    render_to_terminal(commands, terminal);

    getchar();

    free(clay_memory);
    return 0;
}
