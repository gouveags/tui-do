#ifndef TUI_DO_TERMINAL_H
#define TUI_DO_TERMINAL_H

#include "../vendor/clay/clay.h"

typedef struct TerminalSize {
    int width;
    int height;
} TerminalSize;

TerminalSize terminal_get_size(void);
void terminal_enter_fullscreen(void);
void terminal_leave_fullscreen(void);
void terminal_enable_raw_mode(void);
void terminal_disable_raw_mode(void);
int terminal_read_key(void);
void terminal_render(Clay_RenderCommandArray commands, TerminalSize terminal);

#endif
