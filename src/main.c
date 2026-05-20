#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLAY_IMPLEMENTATION
#include "../vendor/clay/clay.h"

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 24

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

static void draw_text(char screen[TERMINAL_HEIGHT][TERMINAL_WIDTH + 1], Clay_RenderCommand *command) {
    Clay_TextRenderData text = command->renderData.text;
    int x = (int)(command->boundingBox.x + 0.5f);
    int y = (int)(command->boundingBox.y + 0.5f);

    if (y < 0 || y >= TERMINAL_HEIGHT) {
        return;
    }

    for (int32_t i = 0; i < text.stringContents.length; i++) {
        int column = x + i;
        if (column >= 0 && column < TERMINAL_WIDTH) {
            screen[y][column] = text.stringContents.chars[i];
        }
    }
}

static void render_to_terminal(Clay_RenderCommandArray commands) {
    char screen[TERMINAL_HEIGHT][TERMINAL_WIDTH + 1];

    for (int y = 0; y < TERMINAL_HEIGHT; y++) {
        memset(screen[y], ' ', TERMINAL_WIDTH);
        screen[y][TERMINAL_WIDTH] = '\0';
    }

    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);

        if (command->commandType == CLAY_RENDER_COMMAND_TYPE_TEXT) {
            draw_text(screen, command);
        }
    }

    printf("\033[2J\033[H");
    for (int y = 0; y < TERMINAL_HEIGHT; y++) {
        puts(screen[y]);
    }
}

int main(void) {
    uint32_t clay_memory_size = Clay_MinMemorySize();
    void *clay_memory = malloc(clay_memory_size);
    if (clay_memory == NULL) {
        fprintf(stderr, "Failed to allocate Clay arena.\n");
        return 1;
    }

    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(clay_memory_size, clay_memory);
    Clay_Initialize(
        arena,
        (Clay_Dimensions) { .width = TERMINAL_WIDTH, .height = TERMINAL_HEIGHT },
        (Clay_ErrorHandler) { .errorHandlerFunction = handle_clay_error }
    );
    Clay_SetMeasureTextFunction(measure_text, NULL);

    Clay_SetLayoutDimensions((Clay_Dimensions) { .width = TERMINAL_WIDTH, .height = TERMINAL_HEIGHT });
    Clay_BeginLayout();

    CLAY(CLAY_ID("Root"), {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
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
    }

    Clay_RenderCommandArray commands = Clay_EndLayout(0.0f);
    render_to_terminal(commands);

    free(clay_memory);
    return 0;
}
