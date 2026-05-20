#include "test.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define CLAY_IMPLEMENTATION
#include "../vendor/clay/clay.h"

#include "../src/terminal.h"

static int text_contains(const char *text, const char *needle) {
    return strstr(text, needle) != NULL;
}

SCENARIO(render_output_clears_the_whole_screen_before_drawing) {
    GIVEN("a terminal that is being redrawn after a resize");

    WHEN("the terminal render prefix is requested");
    const char *prefix = terminal_render_prefix();

    THEN("the prefix clears stale content and returns to the top-left corner");
    EXPECT_TRUE(text_contains(prefix, "\033[2J"));
    EXPECT_TRUE(text_contains(prefix, "\033[H"));
}

SCENARIO(fullscreen_entry_requests_modified_key_reporting) {
    GIVEN("the app is entering fullscreen raw terminal mode");

    WHEN("the fullscreen control sequence is requested");
    const char *sequence = terminal_enter_fullscreen_sequence();

    THEN("modified key reporting is requested so Shift+Enter can be distinguished");
    EXPECT_TRUE(text_contains(sequence, "\033[>4;1m"));
    EXPECT_TRUE(text_contains(sequence, "\033[?1000h"));
    EXPECT_TRUE(text_contains(sequence, "\033[?1006h"));
    EXPECT_TRUE(text_contains(terminal_leave_fullscreen_sequence(), "\033[?1006l"));
    EXPECT_TRUE(text_contains(terminal_leave_fullscreen_sequence(), "\033[?1000l"));
    EXPECT_TRUE(text_contains(terminal_leave_fullscreen_sequence(), "\033[>4;0m"));
}

SCENARIO(key_polling_can_return_without_input) {
    int pipe_fds[2];
    EXPECT_INT_EQ(pipe(pipe_fds), 0);

    GIVEN("there is no key waiting on the input file descriptor");
    int flags = fcntl(pipe_fds[0], F_GETFL, 0);
    EXPECT_TRUE(flags >= 0);
    EXPECT_INT_EQ(fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK), 0);

    WHEN("the app polls for input");
    int key = terminal_try_read_key_from_fd(pipe_fds[0]);

    THEN("the poll returns immediately without a key");
    EXPECT_INT_EQ(key, TERMINAL_KEY_NONE);

    close(pipe_fds[0]);
    close(pipe_fds[1]);
}

SCENARIO(key_polling_reads_complete_modified_key_sequences) {
    int pipe_fds[2];
    const unsigned char shift_enter[] = {27, '[', '1', '3', ';', '2', 'u'};
    EXPECT_INT_EQ(pipe(pipe_fds), 0);

    GIVEN("a full Shift+Enter sequence is waiting");
    EXPECT_INT_EQ(write(pipe_fds[1], shift_enter, sizeof(shift_enter)), (int)sizeof(shift_enter));

    WHEN("the app polls for one key");
    int key = terminal_try_read_key_from_fd(pipe_fds[0]);

    THEN("the entire sequence is decoded as Shift+Enter");
    EXPECT_INT_EQ(key, TERMINAL_KEY_SHIFT_ENTER);

    close(pipe_fds[0]);
    close(pipe_fds[1]);
}

SCENARIO(terminal_decodes_arrow_keys_and_ctrl_c) {
    const unsigned char up[] = {27, '[', 'A'};
    const unsigned char down[] = {27, '[', 'B'};
    const unsigned char right[] = {27, '[', 'C'};
    const unsigned char left[] = {27, '[', 'D'};
    const unsigned char escape[] = {27};
    const unsigned char shift_enter_csi_u[] = {27, '[', '1', '3', ';', '2', 'u'};
    const unsigned char shift_enter_modify_other_keys[] = {27, '[', '2', '7', ';', '2', ';', '1', '3', '~'};
    const unsigned char page_up[] = {27, '[', '5', '~'};
    const unsigned char page_down[] = {27, '[', '6', '~'};
    const unsigned char wheel_up[] = {27, '[', '<', '6', '4', ';', '2', '0', ';', '1', '0', 'M'};
    const unsigned char wheel_down[] = {27, '[', '<', '6', '5', ';', '2', '0', ';', '1', '0', 'M'};
    const unsigned char ctrl_c[] = {3};
    const unsigned char ctrl_s[] = {19};

    GIVEN("raw terminal key bytes");

    WHEN("the bytes are decoded");

    THEN("navigation and termination keys are semantic");
    EXPECT_INT_EQ(terminal_decode_key_sequence(up, 3), TERMINAL_KEY_UP);
    EXPECT_INT_EQ(terminal_decode_key_sequence(down, 3), TERMINAL_KEY_DOWN);
    EXPECT_INT_EQ(terminal_decode_key_sequence(left, 3), TERMINAL_KEY_LEFT);
    EXPECT_INT_EQ(terminal_decode_key_sequence(right, 3), TERMINAL_KEY_RIGHT);
    EXPECT_INT_EQ(terminal_decode_key_sequence(escape, 1), TERMINAL_KEY_ESCAPE);
    EXPECT_INT_EQ(terminal_decode_key_sequence(shift_enter_csi_u, 7), TERMINAL_KEY_SHIFT_ENTER);
    EXPECT_INT_EQ(terminal_decode_key_sequence(shift_enter_modify_other_keys, 10), TERMINAL_KEY_SHIFT_ENTER);
    EXPECT_INT_EQ(terminal_decode_key_sequence(page_up, 4), TERMINAL_KEY_PAGE_UP);
    EXPECT_INT_EQ(terminal_decode_key_sequence(page_down, 4), TERMINAL_KEY_PAGE_DOWN);
    EXPECT_INT_EQ(terminal_decode_key_sequence(wheel_up, 12), TERMINAL_KEY_MOUSE_WHEEL_UP);
    EXPECT_INT_EQ(terminal_decode_key_sequence(wheel_down, 12), TERMINAL_KEY_MOUSE_WHEEL_DOWN);
    EXPECT_INT_EQ(terminal_decode_key_sequence(ctrl_c, 1), TERMINAL_KEY_CTRL_C);
    EXPECT_INT_EQ(terminal_decode_key_sequence(ctrl_s, 1), TERMINAL_KEY_CTRL_S);
}

int main(void) {
    RUN_SCENARIO(render_output_clears_the_whole_screen_before_drawing);
    RUN_SCENARIO(fullscreen_entry_requests_modified_key_reporting);
    RUN_SCENARIO(key_polling_can_return_without_input);
    RUN_SCENARIO(key_polling_reads_complete_modified_key_sequences);
    RUN_SCENARIO(terminal_decodes_arrow_keys_and_ctrl_c);

    return finish_tests();
}
