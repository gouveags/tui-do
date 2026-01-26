import assert from "node:assert";
import { describe, it } from "node:test";
import { ANSI, cursor, screen, style } from "../../src/render/ansi.ts";

describe("ANSI escape codes", () => {
  it("defines ESC as \\x1b", () => {
    assert.strictEqual(ANSI.ESC, "\x1b");
  });

  it("defines CSI as ESC[", () => {
    assert.strictEqual(ANSI.CSI, "\x1b[");
  });
});

describe("cursor", () => {
  it("hides cursor", () => {
    assert.strictEqual(cursor.hide, "\x1b[?25l");
  });

  it("shows cursor", () => {
    assert.strictEqual(cursor.show, "\x1b[?25h");
  });

  it("moves cursor to position", () => {
    assert.strictEqual(cursor.moveTo(1, 1), "\x1b[1;1H");
    assert.strictEqual(cursor.moveTo(5, 10), "\x1b[5;10H");
  });

  it("moves cursor home", () => {
    assert.strictEqual(cursor.home, "\x1b[H");
  });
});

describe("style", () => {
  it("resets style", () => {
    assert.strictEqual(style.reset, "\x1b[0m");
  });

  it("applies bold", () => {
    assert.strictEqual(style.bold, "\x1b[1m");
  });

  it("applies dim", () => {
    assert.strictEqual(style.dim, "\x1b[2m");
  });

  it("applies inverse", () => {
    assert.strictEqual(style.inverse, "\x1b[7m");
  });

  it("applies foreground colors", () => {
    assert.strictEqual(style.fg.black, "\x1b[30m");
    assert.strictEqual(style.fg.red, "\x1b[31m");
    assert.strictEqual(style.fg.green, "\x1b[32m");
    assert.strictEqual(style.fg.yellow, "\x1b[33m");
    assert.strictEqual(style.fg.blue, "\x1b[34m");
    assert.strictEqual(style.fg.magenta, "\x1b[35m");
    assert.strictEqual(style.fg.cyan, "\x1b[36m");
    assert.strictEqual(style.fg.white, "\x1b[37m");
  });

  it("applies background colors", () => {
    assert.strictEqual(style.bg.black, "\x1b[40m");
    assert.strictEqual(style.bg.red, "\x1b[41m");
    assert.strictEqual(style.bg.green, "\x1b[42m");
    assert.strictEqual(style.bg.yellow, "\x1b[43m");
    assert.strictEqual(style.bg.blue, "\x1b[44m");
    assert.strictEqual(style.bg.magenta, "\x1b[45m");
    assert.strictEqual(style.bg.cyan, "\x1b[46m");
    assert.strictEqual(style.bg.white, "\x1b[47m");
  });
});

describe("screen", () => {
  it("clears entire screen", () => {
    assert.strictEqual(screen.clear, "\x1b[2J");
  });

  it("clears line", () => {
    assert.strictEqual(screen.clearLine, "\x1b[2K");
  });

  it("enables alternate buffer", () => {
    assert.strictEqual(screen.altBuffer, "\x1b[?1049h");
  });

  it("disables alternate buffer", () => {
    assert.strictEqual(screen.mainBuffer, "\x1b[?1049l");
  });
});
