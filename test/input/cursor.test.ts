import assert from "node:assert";
import { describe, it } from "node:test";
import {
  createInputBuffer,
  deleteBackward,
  deleteForward,
  getLineText,
  getTotalLength,
  insertChar,
  insertNewline,
  moveDown,
  moveLeft,
  moveRight,
  moveToLineEnd,
  moveToLineStart,
  moveUp,
} from "../../src/input/cursor.ts";
import type { InputBuffer } from "../../src/state/types.ts";

describe("cursor navigation", () => {
  describe("moveLeft", () => {
    it("moves cursor left within line", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 3 },
      };
      const next = moveLeft(buf);
      assert.strictEqual(next.cursor.col, 2);
    });

    it("moves to end of previous line at start of line", () => {
      const buf: InputBuffer = {
        lines: ["hello", "world"],
        cursor: { line: 1, col: 0 },
      };
      const next = moveLeft(buf);
      assert.strictEqual(next.cursor.line, 0);
      assert.strictEqual(next.cursor.col, 5);
    });

    it("stays at beginning of first line", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 0 },
      };
      const next = moveLeft(buf);
      assert.strictEqual(next.cursor.line, 0);
      assert.strictEqual(next.cursor.col, 0);
    });
  });

  describe("moveRight", () => {
    it("moves cursor right within line", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 2 },
      };
      const next = moveRight(buf);
      assert.strictEqual(next.cursor.col, 3);
    });

    it("moves to start of next line at end of line", () => {
      const buf: InputBuffer = {
        lines: ["hello", "world"],
        cursor: { line: 0, col: 5 },
      };
      const next = moveRight(buf);
      assert.strictEqual(next.cursor.line, 1);
      assert.strictEqual(next.cursor.col, 0);
    });

    it("stays at end of last line", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 5 },
      };
      const next = moveRight(buf);
      assert.strictEqual(next.cursor.col, 5);
    });
  });

  describe("moveUp", () => {
    it("moves cursor up preserving column", () => {
      const buf: InputBuffer = {
        lines: ["hello", "world"],
        cursor: { line: 1, col: 3 },
      };
      const next = moveUp(buf);
      assert.strictEqual(next.cursor.line, 0);
      assert.strictEqual(next.cursor.col, 3);
    });

    it("clamps column to shorter line", () => {
      const buf: InputBuffer = {
        lines: ["hi", "world"],
        cursor: { line: 1, col: 4 },
      };
      const next = moveUp(buf);
      assert.strictEqual(next.cursor.line, 0);
      assert.strictEqual(next.cursor.col, 2);
    });

    it("stays on first line", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 2 },
      };
      const next = moveUp(buf);
      assert.strictEqual(next.cursor.line, 0);
    });
  });

  describe("moveDown", () => {
    it("moves cursor down preserving column", () => {
      const buf: InputBuffer = {
        lines: ["hello", "world"],
        cursor: { line: 0, col: 3 },
      };
      const next = moveDown(buf);
      assert.strictEqual(next.cursor.line, 1);
      assert.strictEqual(next.cursor.col, 3);
    });

    it("clamps column to shorter line", () => {
      const buf: InputBuffer = {
        lines: ["hello", "hi"],
        cursor: { line: 0, col: 4 },
      };
      const next = moveDown(buf);
      assert.strictEqual(next.cursor.line, 1);
      assert.strictEqual(next.cursor.col, 2);
    });

    it("stays on last line", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 2 },
      };
      const next = moveDown(buf);
      assert.strictEqual(next.cursor.line, 0);
    });
  });

  describe("moveToLineStart", () => {
    it("moves cursor to beginning of line", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 3 },
      };
      const next = moveToLineStart(buf);
      assert.strictEqual(next.cursor.col, 0);
    });
  });

  describe("moveToLineEnd", () => {
    it("moves cursor to end of line", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 2 },
      };
      const next = moveToLineEnd(buf);
      assert.strictEqual(next.cursor.col, 5);
    });
  });
});

describe("text editing", () => {
  describe("insertChar", () => {
    it("inserts character at cursor position", () => {
      const buf: InputBuffer = { lines: ["hllo"], cursor: { line: 0, col: 1 } };
      const next = insertChar(buf, "e");
      assert.strictEqual(next.lines[0], "hello");
      assert.strictEqual(next.cursor.col, 2);
    });

    it("inserts at end of line", () => {
      const buf: InputBuffer = { lines: ["hell"], cursor: { line: 0, col: 4 } };
      const next = insertChar(buf, "o");
      assert.strictEqual(next.lines[0], "hello");
      assert.strictEqual(next.cursor.col, 5);
    });
  });

  describe("deleteBackward", () => {
    it("deletes character before cursor", () => {
      const buf: InputBuffer = {
        lines: ["helllo"],
        cursor: { line: 0, col: 4 },
      };
      const next = deleteBackward(buf);
      assert.strictEqual(next.lines[0], "hello");
      assert.strictEqual(next.cursor.col, 3);
    });

    it("joins lines when at start of line", () => {
      const buf: InputBuffer = {
        lines: ["hello", "world"],
        cursor: { line: 1, col: 0 },
      };
      const next = deleteBackward(buf);
      assert.strictEqual(next.lines.length, 1);
      assert.strictEqual(next.lines[0], "helloworld");
      assert.strictEqual(next.cursor.line, 0);
      assert.strictEqual(next.cursor.col, 5);
    });

    it("does nothing at start of buffer", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 0 },
      };
      const next = deleteBackward(buf);
      assert.strictEqual(next.lines[0], "hello");
      assert.strictEqual(next.cursor.col, 0);
    });
  });

  describe("deleteForward", () => {
    it("deletes character after cursor", () => {
      const buf: InputBuffer = {
        lines: ["helllo"],
        cursor: { line: 0, col: 3 },
      };
      const next = deleteForward(buf);
      assert.strictEqual(next.lines[0], "hello");
      assert.strictEqual(next.cursor.col, 3);
    });

    it("joins lines when at end of line", () => {
      const buf: InputBuffer = {
        lines: ["hello", "world"],
        cursor: { line: 0, col: 5 },
      };
      const next = deleteForward(buf);
      assert.strictEqual(next.lines.length, 1);
      assert.strictEqual(next.lines[0], "helloworld");
    });

    it("does nothing at end of buffer", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 5 },
      };
      const next = deleteForward(buf);
      assert.strictEqual(next.lines[0], "hello");
    });
  });

  describe("insertNewline", () => {
    it("splits line at cursor position", () => {
      const buf: InputBuffer = {
        lines: ["helloworld"],
        cursor: { line: 0, col: 5 },
      };
      const next = insertNewline(buf);
      assert.strictEqual(next.lines.length, 2);
      assert.strictEqual(next.lines[0], "hello");
      assert.strictEqual(next.lines[1], "world");
      assert.strictEqual(next.cursor.line, 1);
      assert.strictEqual(next.cursor.col, 0);
    });

    it("creates empty line at end", () => {
      const buf: InputBuffer = {
        lines: ["hello"],
        cursor: { line: 0, col: 5 },
      };
      const next = insertNewline(buf);
      assert.strictEqual(next.lines.length, 2);
      assert.strictEqual(next.lines[1], "");
    });
  });
});

describe("utility functions", () => {
  describe("createInputBuffer", () => {
    it("creates empty buffer", () => {
      const buf = createInputBuffer();
      assert.strictEqual(buf.lines.length, 1);
      assert.strictEqual(buf.lines[0], "");
      assert.strictEqual(buf.cursor.line, 0);
      assert.strictEqual(buf.cursor.col, 0);
    });

    it("creates buffer with initial text", () => {
      const buf = createInputBuffer("hello");
      assert.strictEqual(buf.lines[0], "hello");
      assert.strictEqual(buf.cursor.col, 5);
    });
  });

  describe("getLineText", () => {
    it("returns current line text", () => {
      const buf: InputBuffer = {
        lines: ["hello", "world"],
        cursor: { line: 1, col: 2 },
      };
      assert.strictEqual(getLineText(buf), "world");
    });
  });

  describe("getTotalLength", () => {
    it("returns total character count", () => {
      const buf: InputBuffer = {
        lines: ["hello", "world"],
        cursor: { line: 0, col: 0 },
      };
      assert.strictEqual(getTotalLength(buf), 11);
    });
  });
});
