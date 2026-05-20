#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CLAY_IMPLEMENTATION
#include "../vendor/clay/clay.h"

#include "terminal.h"
#include "ui.h"
#include "app.h"

static volatile sig_atomic_t terminal_was_resized = 0;

static void handle_resize_signal(int signal_number) {
    (void)signal_number;
    terminal_was_resized = 1;
}

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
    signal(SIGWINCH, handle_resize_signal);

    TerminalSize terminal = terminal_get_size();
    if (terminal.width <= 0 || terminal.height <= 0) {
        fprintf(stderr, "Could not determine terminal size.\n");
        return 1;
    }

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

    App app = app_create(terminal);

    while (1) {
        if (terminal_was_resized) {
            terminal_was_resized = 0;
            app_observe_terminal_size(&app, terminal_get_size());
        }

        app_observe_terminal_size(&app, terminal_get_size());

        if (app.needs_redraw) {
            Clay_RenderCommandArray commands = ui_render_app(&app.state, app.terminal);
            terminal_render(commands, app.terminal);
            app.needs_redraw = 0;
        }

        int key = terminal_try_read_key();
        if (key == TERMINAL_KEY_NONE) {
            struct timespec frame_pause = {
                .tv_sec = 0,
                .tv_nsec = 16000000,
            };
            nanosleep(&frame_pause, NULL);
        } else {
            app_handle_key(&app, key);
            if (app.should_quit) {
                break;
            }
        }
    }

    free(clay_memory);
    return 0;
}
