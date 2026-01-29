import assert from "node:assert";
import { describe, it } from "node:test";
import { parseKey } from "../../src/input/keys.ts";

describe("parseKey", () => {
  it("parses regular characters", () => {
    const key = parseKey(Buffer.from("a"));
    assert.deepStrictEqual(key, {
      name: "a",
      ctrl: false,
      shift: false,
      sequence: "a",
    });
  });

  it("parses number keys", () => {
    const key1 = parseKey(Buffer.from("1"));
    assert.deepStrictEqual(key1, {
      name: "1",
      ctrl: false,
      shift: false,
      sequence: "1",
    });

    const key2 = parseKey(Buffer.from("3"));
    assert.deepStrictEqual(key2, {
      name: "3",
      ctrl: false,
      shift: false,
      sequence: "3",
    });
  });

  it("parses Enter key (CR)", () => {
    const key = parseKey(Buffer.from("\r"));
    assert.deepStrictEqual(key, {
      name: "enter",
      ctrl: false,
      shift: false,
      sequence: "\r",
    });
  });

  it("parses Enter key (LF) for Windows compatibility", () => {
    const key = parseKey(Buffer.from("\n"));
    assert.deepStrictEqual(key, {
      name: "enter",
      ctrl: false,
      shift: false,
      sequence: "\n",
    });
  });

  it("parses Enter key (CR+LF) for Windows compatibility", () => {
    const key = parseKey(Buffer.from("\r\n"));
    assert.deepStrictEqual(key, {
      name: "enter",
      ctrl: false,
      shift: false,
      sequence: "\r\n",
    });
  });

  it("parses Escape key", () => {
    const key = parseKey(Buffer.from("\x1b"));
    assert.deepStrictEqual(key, {
      name: "escape",
      ctrl: false,
      shift: false,
      sequence: "\x1b",
    });
  });

  it("parses Ctrl+C", () => {
    const key = parseKey(Buffer.from("\x03"));
    assert.deepStrictEqual(key, {
      name: "c",
      ctrl: true,
      shift: false,
      sequence: "\x03",
    });
  });

  it("parses Ctrl+D", () => {
    const key = parseKey(Buffer.from("\x04"));
    assert.deepStrictEqual(key, {
      name: "d",
      ctrl: true,
      shift: false,
      sequence: "\x04",
    });
  });

  it("parses up arrow", () => {
    const key = parseKey(Buffer.from("\x1b[A"));
    assert.deepStrictEqual(key, {
      name: "up",
      ctrl: false,
      shift: false,
      sequence: "\x1b[A",
    });
  });

  it("parses down arrow", () => {
    const key = parseKey(Buffer.from("\x1b[B"));
    assert.deepStrictEqual(key, {
      name: "down",
      ctrl: false,
      shift: false,
      sequence: "\x1b[B",
    });
  });

  it("parses right arrow", () => {
    const key = parseKey(Buffer.from("\x1b[C"));
    assert.deepStrictEqual(key, {
      name: "right",
      ctrl: false,
      shift: false,
      sequence: "\x1b[C",
    });
  });

  it("parses left arrow", () => {
    const key = parseKey(Buffer.from("\x1b[D"));
    assert.deepStrictEqual(key, {
      name: "left",
      ctrl: false,
      shift: false,
      sequence: "\x1b[D",
    });
  });

  it("parses backspace", () => {
    const key = parseKey(Buffer.from("\x7f"));
    assert.deepStrictEqual(key, {
      name: "backspace",
      ctrl: false,
      shift: false,
      sequence: "\x7f",
    });
  });

  it("parses tab", () => {
    const key = parseKey(Buffer.from("\t"));
    assert.deepStrictEqual(key, {
      name: "tab",
      ctrl: false,
      shift: false,
      sequence: "\t",
    });
  });

  it("parses shift+enter", () => {
    const key = parseKey(Buffer.from("\x1b[13;2u"));
    assert.deepStrictEqual(key, {
      name: "shift-enter",
      ctrl: false,
      shift: true,
      sequence: "\x1b[13;2u",
    });
  });

  it("parses ctrl+up arrow", () => {
    const key = parseKey(Buffer.from("\x1b[1;5A"));
    assert.deepStrictEqual(key, {
      name: "ctrl-up",
      ctrl: true,
      shift: false,
      sequence: "\x1b[1;5A",
    });
  });

  it("parses ctrl+down arrow", () => {
    const key = parseKey(Buffer.from("\x1b[1;5B"));
    assert.deepStrictEqual(key, {
      name: "ctrl-down",
      ctrl: true,
      shift: false,
      sequence: "\x1b[1;5B",
    });
  });

  it("returns unknown for unrecognized sequences", () => {
    const key = parseKey(Buffer.from("\x1b[Z"));
    assert.strictEqual(key.name, "unknown");
  });

  // Kitty keyboard protocol CSI u format tests
  describe("Kitty protocol CSI u format", () => {
    it("parses escape key in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[27u"));
      assert.deepStrictEqual(key, {
        name: "escape",
        ctrl: false,
        shift: false,
        sequence: "\x1b[27u",
      });
    });

    it("parses escape key with modifier in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[27;1u"));
      assert.deepStrictEqual(key, {
        name: "escape",
        ctrl: false,
        shift: false,
        sequence: "\x1b[27;1u",
      });
    });

    it("parses up arrow in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57352u"));
      assert.deepStrictEqual(key, {
        name: "up",
        ctrl: false,
        shift: false,
        sequence: "\x1b[57352u",
      });
    });

    it("parses down arrow in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57353u"));
      assert.deepStrictEqual(key, {
        name: "down",
        ctrl: false,
        shift: false,
        sequence: "\x1b[57353u",
      });
    });

    it("parses left arrow in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57350u"));
      assert.deepStrictEqual(key, {
        name: "left",
        ctrl: false,
        shift: false,
        sequence: "\x1b[57350u",
      });
    });

    it("parses right arrow in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57351u"));
      assert.deepStrictEqual(key, {
        name: "right",
        ctrl: false,
        shift: false,
        sequence: "\x1b[57351u",
      });
    });

    it("parses enter in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[13u"));
      assert.deepStrictEqual(key, {
        name: "enter",
        ctrl: false,
        shift: false,
        sequence: "\x1b[13u",
      });
    });

    it("parses shift+enter in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[13;2u"));
      assert.deepStrictEqual(key, {
        name: "shift-enter",
        ctrl: false,
        shift: true,
        sequence: "\x1b[13;2u",
      });
    });

    it("parses ctrl+c in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[99;5u"));
      assert.deepStrictEqual(key, {
        name: "c",
        ctrl: true,
        shift: false,
        sequence: "\x1b[99;5u",
      });
    });

    it("parses ctrl+up in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57352;5u"));
      assert.deepStrictEqual(key, {
        name: "ctrl-up",
        ctrl: true,
        shift: false,
        sequence: "\x1b[57352;5u",
      });
    });

    it("parses ctrl+down in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57353;5u"));
      assert.deepStrictEqual(key, {
        name: "ctrl-down",
        ctrl: true,
        shift: false,
        sequence: "\x1b[57353;5u",
      });
    });

    it("parses backspace in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[127u"));
      assert.deepStrictEqual(key, {
        name: "backspace",
        ctrl: false,
        shift: false,
        sequence: "\x1b[127u",
      });
    });

    it("parses delete in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57359u"));
      assert.deepStrictEqual(key, {
        name: "delete",
        ctrl: false,
        shift: false,
        sequence: "\x1b[57359u",
      });
    });

    it("parses home in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57356u"));
      assert.deepStrictEqual(key, {
        name: "home",
        ctrl: false,
        shift: false,
        sequence: "\x1b[57356u",
      });
    });

    it("parses end in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[57357u"));
      assert.deepStrictEqual(key, {
        name: "end",
        ctrl: false,
        shift: false,
        sequence: "\x1b[57357u",
      });
    });

    it("parses tab in CSI u format", () => {
      const key = parseKey(Buffer.from("\x1b[9u"));
      assert.deepStrictEqual(key, {
        name: "tab",
        ctrl: false,
        shift: false,
        sequence: "\x1b[9u",
      });
    });
  });
});
