#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CLAY_IMPLEMENTATION
#include "../vendor/clay/clay.h"

#include "terminal.h"
#include "ui.h"

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

int main(void) {
    terminal_enter_fullscreen();
    terminal_enable_raw_mode();
    atexit(terminal_disable_raw_mode);
    atexit(terminal_leave_fullscreen);

    TerminalSize terminal = terminal_get_size();
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

    AppState state = {
        .selected_menu_index = 0,
    };
    Clay_RenderCommandArray commands = ui_render_main_menu(&state, terminal);
    terminal_render(commands, terminal);

    while (terminal_read_key() != 'q') {
    }

    free(clay_memory);
    return 0;
}
