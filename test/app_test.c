#include "test.h"

#include "../src/app.h"

SCENARIO(app_marks_itself_for_redraw_when_the_terminal_resizes) {
    App app = app_create((TerminalSize) { .width = 160, .height = 50 });
    app.needs_redraw = 0;

    GIVEN("an app already rendered at a large terminal size");

    WHEN("the terminal is resized smaller");
    app_handle_resize(&app, (TerminalSize) { .width = 72, .height = 20 });

    THEN("the app stores the exact new size and schedules a redraw");
    EXPECT_INT_EQ(app.terminal.width, 72);
    EXPECT_INT_EQ(app.terminal.height, 20);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(app_does_not_redraw_when_the_terminal_size_is_unchanged) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.needs_redraw = 0;

    GIVEN("an app rendered at the current terminal size");

    WHEN("resize handling observes the same size again");
    app_handle_resize(&app, (TerminalSize) { .width = 100, .height = 30 });

    THEN("no extra redraw is scheduled");
    EXPECT_INT_EQ(app.needs_redraw, 0);
}

SCENARIO(app_waits_for_resize_to_stabilize_before_redrawing) {
    App app = app_create((TerminalSize) { .width = 120, .height = 40 });
    app.needs_redraw = 0;

    GIVEN("a resize reports an intermediate terminal size");

    WHEN("the app observes that size for the first time");
    app_observe_terminal_size(&app, (TerminalSize) { .width = 90, .height = 24 });

    THEN("the current layout size is not replaced yet");
    EXPECT_INT_EQ(app.terminal.width, 120);
    EXPECT_INT_EQ(app.terminal.height, 40);
    EXPECT_INT_EQ(app.needs_redraw, 0);

    WHEN("the same size is observed again");
    app_observe_terminal_size(&app, (TerminalSize) { .width = 90, .height = 24 });

    THEN("the resize is treated as stable and the app redraws");
    EXPECT_INT_EQ(app.terminal.width, 90);
    EXPECT_INT_EQ(app.terminal.height, 24);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

SCENARIO(app_uses_the_latest_size_after_noisy_resize_events) {
    App app = app_create((TerminalSize) { .width = 100, .height = 30 });
    app.needs_redraw = 0;

    GIVEN("multiple transient sizes are observed during one resize");

    WHEN("the final full-screen size is the one that stabilizes");
    app_observe_terminal_size(&app, (TerminalSize) { .width = 72, .height = 20 });
    app_observe_terminal_size(&app, (TerminalSize) { .width = 160, .height = 48 });
    app_observe_terminal_size(&app, (TerminalSize) { .width = 160, .height = 48 });

    THEN("the app adopts the latest stable size, not the first transient one");
    EXPECT_INT_EQ(app.terminal.width, 160);
    EXPECT_INT_EQ(app.terminal.height, 48);
    EXPECT_INT_EQ(app.needs_redraw, 1);
}

int main(void) {
    RUN_SCENARIO(app_marks_itself_for_redraw_when_the_terminal_resizes);
    RUN_SCENARIO(app_does_not_redraw_when_the_terminal_size_is_unchanged);
    RUN_SCENARIO(app_waits_for_resize_to_stabilize_before_redrawing);
    RUN_SCENARIO(app_uses_the_latest_size_after_noisy_resize_events);

    return finish_tests();
}
