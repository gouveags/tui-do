import assert from "node:assert";
import { describe, it } from "node:test";
import { parseKey } from "../../src/input/keys.ts";

describe("parseKey", () => {
  it("parses regular characters", () => {
    const key = parseKey(Buffer.from("a"));
    assert.deepStrictEqual(key, { name: "a", ctrl: false, sequence: "a" });
  });

  it("parses number keys", () => {
    const key1 = parseKey(Buffer.from("1"));
    assert.deepStrictEqual(key1, { name: "1", ctrl: false, sequence: "1" });

    const key2 = parseKey(Buffer.from("3"));
    assert.deepStrictEqual(key2, { name: "3", ctrl: false, sequence: "3" });
  });

  it("parses Enter key", () => {
    const key = parseKey(Buffer.from("\r"));
    assert.deepStrictEqual(key, { name: "enter", ctrl: false, sequence: "\r" });
  });

  it("parses Escape key", () => {
    const key = parseKey(Buffer.from("\x1b"));
    assert.deepStrictEqual(key, {
      name: "escape",
      ctrl: false,
      sequence: "\x1b",
    });
  });

  it("parses Ctrl+C", () => {
    const key = parseKey(Buffer.from("\x03"));
    assert.deepStrictEqual(key, { name: "c", ctrl: true, sequence: "\x03" });
  });

  it("parses Ctrl+D", () => {
    const key = parseKey(Buffer.from("\x04"));
    assert.deepStrictEqual(key, { name: "d", ctrl: true, sequence: "\x04" });
  });

  it("parses up arrow", () => {
    const key = parseKey(Buffer.from("\x1b[A"));
    assert.deepStrictEqual(key, {
      name: "up",
      ctrl: false,
      sequence: "\x1b[A",
    });
  });

  it("parses down arrow", () => {
    const key = parseKey(Buffer.from("\x1b[B"));
    assert.deepStrictEqual(key, {
      name: "down",
      ctrl: false,
      sequence: "\x1b[B",
    });
  });

  it("parses right arrow", () => {
    const key = parseKey(Buffer.from("\x1b[C"));
    assert.deepStrictEqual(key, {
      name: "right",
      ctrl: false,
      sequence: "\x1b[C",
    });
  });

  it("parses left arrow", () => {
    const key = parseKey(Buffer.from("\x1b[D"));
    assert.deepStrictEqual(key, {
      name: "left",
      ctrl: false,
      sequence: "\x1b[D",
    });
  });

  it("parses backspace", () => {
    const key = parseKey(Buffer.from("\x7f"));
    assert.deepStrictEqual(key, {
      name: "backspace",
      ctrl: false,
      sequence: "\x7f",
    });
  });

  it("parses tab", () => {
    const key = parseKey(Buffer.from("\t"));
    assert.deepStrictEqual(key, { name: "tab", ctrl: false, sequence: "\t" });
  });

  it("returns unknown for unrecognized sequences", () => {
    const key = parseKey(Buffer.from("\x1b[Z"));
    assert.strictEqual(key.name, "unknown");
  });
});
