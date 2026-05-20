#include "terminal.h"

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define FALLBACK_TERMINAL_WIDTH 80
#define FALLBACK_TERMINAL_HEIGHT 24

typedef struct TerminalCell {
    char value;
    Clay_Color foreground;
    Clay_Color background;
} TerminalCell;

static struct termios original_terminal_mode;
static int raw_mode_enabled = 0;

static int read_terminal_size(int fd, TerminalSize *terminal) {
    struct winsize size;

    if (ioctl(fd, TIOCGWINSZ, &size) == 0 && size.ws_col > 0 && size.ws_row > 0) {
        terminal->width = size.ws_col;
        terminal->height = size.ws_row;
        return 1;
    }

    return 0;
}

TerminalSize terminal_get_size(void) {
    TerminalSize terminal = {
        .width = FALLBACK_TERMINAL_WIDTH,
        .height = FALLBACK_TERMINAL_HEIGHT,
    };
    TerminalSize candidate;
    int tty_fd;

    if (
        read_terminal_size(STDOUT_FILENO, &candidate) ||
        read_terminal_size(STDERR_FILENO, &candidate) ||
        read_terminal_size(STDIN_FILENO, &candidate)
    ) {
        terminal = candidate;
    }

    tty_fd = open("/dev/tty", O_RDONLY);
    if (tty_fd >= 0) {
        if (read_terminal_size(tty_fd, &candidate)) {
            if (candidate.width > terminal.width) {
                terminal.width = candidate.width;
            }
            if (candidate.height > terminal.height) {
                terminal.height = candidate.height;
            }
        }
        close(tty_fd);
    }

    char *columns = getenv("COLUMNS");
    char *lines = getenv("LINES");
    if (columns != NULL && lines != NULL) {
        int width = atoi(columns);
        int height = atoi(lines);
        if (width > 0 && height > 0) {
            if (width > terminal.width) {
                terminal.width = width;
            }
            if (height > terminal.height) {
                terminal.height = height;
            }
        }
    }

    return terminal;
}

void terminal_enter_fullscreen(void) {
    printf("\033[?1049h\033[?25l\033[2J\033[H");
    fflush(stdout);
}

void terminal_leave_fullscreen(void) {
    printf("\033[?25h\033[?1049l");
    fflush(stdout);
}

void terminal_enable_raw_mode(void) {
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &original_terminal_mode) != 0) {
        return;
    }

    raw = original_terminal_mode;
    raw.c_lflag &= (tcflag_t)~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= (tcflag_t)~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= (tcflag_t)~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0) {
        raw_mode_enabled = 1;
    }
}

void terminal_disable_raw_mode(void) {
    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal_mode);
        raw_mode_enabled = 0;
    }
}

int terminal_read_key(void) {
    unsigned char key;
    if (read(STDIN_FILENO, &key, 1) != 1) {
        return -1;
    }
    return key;
}

static TerminalCell *screen_cell(TerminalCell *screen, TerminalSize terminal, int x, int y) {
    return &screen[(y * (terminal.width + 1)) + x];
}

static void draw_rectangle(TerminalCell *screen, TerminalSize terminal, Clay_RenderCommand *command) {
    Clay_Color color = command->renderData.rectangle.backgroundColor;
    int start_x = (int)(command->boundingBox.x + 0.5f);
    int start_y = (int)(command->boundingBox.y + 0.5f);
    int width = (int)(command->boundingBox.width + 0.5f);
    int height = (int)(command->boundingBox.height + 0.5f);

    for (int y = start_y; y < start_y + height; y++) {
        if (y < 0 || y >= terminal.height) {
            continue;
        }
        for (int x = start_x; x < start_x + width; x++) {
            if (x < 0 || x >= terminal.width) {
                continue;
            }
            screen_cell(screen, terminal, x, y)->background = color;
        }
    }
}

static void draw_text(TerminalCell *screen, TerminalSize terminal, Clay_RenderCommand *command) {
    Clay_TextRenderData text = command->renderData.text;
    int x = (int)(command->boundingBox.x + 0.5f);
    int y = (int)(command->boundingBox.y + 0.5f);

    if (y < 0 || y >= terminal.height) {
        return;
    }

    for (int32_t i = 0; i < text.stringContents.length; i++) {
        int column = x + i;
        if (column >= 0 && column < terminal.width) {
            TerminalCell *cell = screen_cell(screen, terminal, column, y);
            cell->value = text.stringContents.chars[i];
            cell->foreground = text.textColor;
        }
    }
}

static int colors_match(Clay_Color a, Clay_Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

static void print_foreground(Clay_Color color) {
    printf("\033[38;2;%d;%d;%dm", (int)color.r, (int)color.g, (int)color.b);
}

static void print_background(Clay_Color color) {
    printf("\033[48;2;%d;%d;%dm", (int)color.r, (int)color.g, (int)color.b);
}

void terminal_render(Clay_RenderCommandArray commands, TerminalSize terminal) {
    size_t row_length = (size_t)terminal.width + 1;
    size_t screen_size = row_length * (size_t)terminal.height;
    TerminalCell *screen = malloc(screen_size * sizeof(TerminalCell));
    Clay_Color default_foreground = {232, 236, 234, 255};
    Clay_Color default_background = {18, 22, 28, 255};

    if (screen == NULL) {
        fprintf(stderr, "Failed to allocate terminal buffer.\n");
        return;
    }

    for (int y = 0; y < terminal.height; y++) {
        for (int x = 0; x <= terminal.width; x++) {
            TerminalCell *cell = screen_cell(screen, terminal, x, y);
            cell->value = x == terminal.width ? '\0' : ' ';
            cell->foreground = default_foreground;
            cell->background = default_background;
        }
    }

    for (int32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);

        if (command->commandType == CLAY_RENDER_COMMAND_TYPE_RECTANGLE) {
            draw_rectangle(screen, terminal, command);
        } else if (command->commandType == CLAY_RENDER_COMMAND_TYPE_TEXT) {
            draw_text(screen, terminal, command);
        }
    }

    printf("\033[H\033[0m");
    for (int y = 0; y < terminal.height; y++) {
        Clay_Color current_foreground = {0, 0, 0, 0};
        Clay_Color current_background = {0, 0, 0, 0};
        for (int x = 0; x < terminal.width; x++) {
            TerminalCell *cell = screen_cell(screen, terminal, x, y);
            if (!colors_match(current_background, cell->background)) {
                print_background(cell->background);
                current_background = cell->background;
            }
            if (!colors_match(current_foreground, cell->foreground)) {
                print_foreground(cell->foreground);
                current_foreground = cell->foreground;
            }
            putchar(cell->value);
        }
        if (y < terminal.height - 1) {
            printf("\r\n");
        }
    }
    printf("\033[0m");
    fflush(stdout);

    free(screen);
}
