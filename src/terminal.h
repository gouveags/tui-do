#ifndef TUI_DO_TERMINAL_H
#define TUI_DO_TERMINAL_H

#include "../vendor/clay/clay.h"

#define TERMINAL_KEY_NONE -1
#define TERMINAL_KEY_UP 1000
#define TERMINAL_KEY_DOWN 1001
#define TERMINAL_KEY_ENTER 1002
#define TERMINAL_KEY_CTRL_C 1003
#define TERMINAL_KEY_BACKSPACE 1004
#define TERMINAL_KEY_LEFT 1005
#define TERMINAL_KEY_RIGHT 1006
#define TERMINAL_KEY_ESCAPE 1007
#define TERMINAL_KEY_SHIFT_ENTER 1008
#define TERMINAL_KEY_CTRL_S 1009

typedef struct TerminalSize {
    int width;
    int height;
} TerminalSize;

TerminalSize terminal_get_size(void);
TerminalSize terminal_resolve_size(TerminalSize stdio_size, TerminalSize tty_size, TerminalSize env_size);
const char *terminal_enter_fullscreen_sequence(void);
const char *terminal_leave_fullscreen_sequence(void);
void terminal_enter_fullscreen(void);
void terminal_leave_fullscreen(void);
void terminal_enable_raw_mode(void);
void terminal_disable_raw_mode(void);
int terminal_read_key(void);
int terminal_try_read_key(void);
int terminal_try_read_key_from_fd(int fd);
int terminal_decode_key_sequence(const unsigned char *bytes, int length);
const char *terminal_render_prefix(void);
void terminal_render(Clay_RenderCommandArray commands, TerminalSize terminal);

#endif
