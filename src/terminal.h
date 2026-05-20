#ifndef TUI_DO_TERMINAL_H
#define TUI_DO_TERMINAL_H

#include "../vendor/clay/clay.h"

#define TERMINAL_KEY_NONE -1

typedef struct TerminalSize {
    int width;
    int height;
} TerminalSize;

TerminalSize terminal_get_size(void);
TerminalSize terminal_resolve_size(TerminalSize stdio_size, TerminalSize tty_size, TerminalSize env_size);
void terminal_enter_fullscreen(void);
void terminal_leave_fullscreen(void);
void terminal_enable_raw_mode(void);
void terminal_disable_raw_mode(void);
int terminal_read_key(void);
int terminal_try_read_key(void);
int terminal_try_read_key_from_fd(int fd);
const char *terminal_render_prefix(void);
void terminal_render(Clay_RenderCommandArray commands, TerminalSize terminal);

#endif
