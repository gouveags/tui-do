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

int main(void) {
    RUN_SCENARIO(render_output_clears_the_whole_screen_before_drawing);
    RUN_SCENARIO(key_polling_can_return_without_input);

    return finish_tests();
}
