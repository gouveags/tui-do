import assert from "node:assert";
import { describe, it } from "node:test";
import { createScreen, getTerminalSize } from "../../src/render/screen.ts";

describe("getTerminalSize", () => {
  it("returns rows and cols", () => {
    const size = getTerminalSize();
    assert.ok(typeof size.rows === "number");
    assert.ok(typeof size.cols === "number");
    assert.ok(size.rows > 0);
    assert.ok(size.cols > 0);
  });
});

describe("Screen", () => {
  it("creates screen with buffer", () => {
    const screen = createScreen();
    assert.ok(screen);
  });

  it("writes text at position", () => {
    const screen = createScreen();
    screen.writeAt(1, 1, "hello");
    const output = screen.flush();
    assert.ok(output.includes("hello"));
    assert.ok(output.includes("\x1b[1;1H"));
  });

  it("writes styled text", () => {
    const screen = createScreen();
    screen.writeStyled(1, 1, "\x1b[1m", "bold text");
    const output = screen.flush();
    assert.ok(output.includes("\x1b[1m"));
    assert.ok(output.includes("bold text"));
    assert.ok(output.includes("\x1b[0m"));
  });

  it("hides cursor, moves home, and clears screen on prepare", () => {
    const screen = createScreen();
    const output = screen.prepare();
    assert.ok(output.includes("\x1b[?25l"));
    assert.ok(output.includes("\x1b[H"));
    assert.ok(output.includes("\x1b[2J"));
  });

  it("shows cursor on restore", () => {
    const screen = createScreen();
    const output = screen.restore();
    assert.ok(output.includes("\x1b[?25h"));
  });
});
