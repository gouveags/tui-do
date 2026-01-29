export type KeyEvent = {
  name: string;
  ctrl: boolean;
  shift: boolean;
  sequence: string;
};

const CTRL_KEYS: Record<number, string> = {
  3: "c",
  4: "d",
  26: "z",
};

const ESCAPE_SEQUENCES: Record<string, string> = {
  "[A": "up",
  "[B": "down",
  "[C": "right",
  "[D": "left",
  "[H": "home",
  "[F": "end",
  "[3~": "delete",
  "[13;2u": "shift-enter",
  "[27;2;13~": "shift-enter",
  "[1;5A": "ctrl-up",
  "[1;5B": "ctrl-down",
};

// Kitty keyboard protocol CSI u format functional key codes
// See: https://sw.kovidgoyal.net/kitty/keyboard-protocol/
const KITTY_FUNCTIONAL_KEYS: Record<number, string> = {
  27: "escape",
  13: "enter",
  9: "tab",
  127: "backspace",
  57350: "left",
  57351: "right",
  57352: "up",
  57353: "down",
  57354: "pageup",
  57355: "pagedown",
  57356: "home",
  57357: "end",
  57358: "insert",
  57359: "delete",
};

// Parse Kitty CSI u format: ESC [ <codepoint> ; <modifiers> u
// or simplified: ESC [ <codepoint> u
const parseKittyCsiU = (suffix: string, seq: string): KeyEvent | null => {
  // Match patterns like "27u", "27;1u", "97;5u" (codepoint;modifiers u)
  const match = suffix.match(/^\[(\d+)(?:;(\d+))?u$/);
  if (!match || !match[1]) return null;

  const codepoint = Number.parseInt(match[1], 10);
  const modifiers = match[2] ? Number.parseInt(match[2], 10) : 1;

  // Decode modifiers (Kitty protocol modifier encoding)
  // modifier - 1 gives a bitmask: bit0=shift, bit1=alt, bit2=ctrl
  // e.g., 1=none, 2=shift, 3=alt, 4=shift+alt, 5=ctrl, 6=ctrl+shift
  const modifierBits = modifiers - 1;
  const shift = (modifierBits & 1) !== 0;
  const ctrl = (modifierBits & 4) !== 0;

  // Check for functional keys first
  const functionalName = KITTY_FUNCTIONAL_KEYS[codepoint];
  if (functionalName) {
    // Handle ctrl+key combinations for functional keys
    if (ctrl && functionalName === "up") {
      return { name: "ctrl-up", ctrl: true, shift, sequence: seq };
    }
    if (ctrl && functionalName === "down") {
      return { name: "ctrl-down", ctrl: true, shift, sequence: seq };
    }
    if (shift && functionalName === "enter") {
      return { name: "shift-enter", ctrl, shift: true, sequence: seq };
    }
    return { name: functionalName, ctrl, shift, sequence: seq };
  }

  // Regular character with modifiers (e.g., Ctrl+C = codepoint 99 with ctrl modifier)
  if (codepoint >= 32 && codepoint < 127) {
    const char = String.fromCharCode(codepoint).toLowerCase();
    return { name: char, ctrl, shift, sequence: seq };
  }

  // Handle control characters sent as codepoints (some terminals do this)
  if (codepoint >= 1 && codepoint <= 26) {
    const name = CTRL_KEYS[codepoint] ?? String.fromCharCode(codepoint + 96);
    return { name, ctrl: true, shift, sequence: seq };
  }

  return null;
};

// Parse legacy CSI format with modifiers: ESC [ 1 ; <modifier> <letter>
// e.g., ESC [ 1 ; 5 A = Ctrl+Up
const parseLegacyModifiedKey = (
  suffix: string,
  seq: string,
): KeyEvent | null => {
  const match = suffix.match(/^\[1;(\d+)([A-Z])$/);
  if (!match || !match[1] || !match[2]) return null;

  const modifier = Number.parseInt(match[1], 10);
  const letter = match[2];

  const ctrl = (modifier - 1) & 4 ? true : false;
  const shift = (modifier - 1) & 1 ? true : false;

  const keyMap: Record<string, string> = {
    A: "up",
    B: "down",
    C: "right",
    D: "left",
    H: "home",
    F: "end",
  };

  const baseName = keyMap[letter];
  if (!baseName) return null;

  if (ctrl && (baseName === "up" || baseName === "down")) {
    return { name: `ctrl-${baseName}`, ctrl: true, shift, sequence: seq };
  }

  return { name: baseName, ctrl, shift, sequence: seq };
};

export const parseKey = (buf: Buffer): KeyEvent => {
  const seq = buf.toString();
  const code = buf[0];

  if (code === undefined) {
    return { name: "unknown", ctrl: false, shift: false, sequence: seq };
  }

  if (code === 0x1b) {
    if (buf.length === 1) {
      return { name: "escape", ctrl: false, shift: false, sequence: seq };
    }

    const suffix = seq.slice(1);

    // Try Kitty CSI u format first (for terminals with Kitty protocol enabled)
    const kittyResult = parseKittyCsiU(suffix, seq);
    if (kittyResult) return kittyResult;

    // Try legacy modified key format (ESC [ 1 ; modifier letter)
    const legacyModifiedResult = parseLegacyModifiedKey(suffix, seq);
    if (legacyModifiedResult) return legacyModifiedResult;

    // Fall back to simple escape sequence lookup
    const name = ESCAPE_SEQUENCES[suffix];
    const isShift = name === "shift-enter";
    const isCtrl = name === "ctrl-up" || name === "ctrl-down";
    return {
      name: name ?? "unknown",
      ctrl: isCtrl,
      shift: isShift,
      sequence: seq,
    };
  }

  if (code === 0x0d) {
    // Handle CR (Linux/macOS Enter) and CR+LF (Windows Enter)
    return { name: "enter", ctrl: false, shift: false, sequence: seq };
  }

  if (code === 0x0a) {
    // Handle LF (Windows terminals sometimes send this for Enter)
    return { name: "enter", ctrl: false, shift: false, sequence: seq };
  }

  if (code === 0x7f) {
    return { name: "backspace", ctrl: false, shift: false, sequence: seq };
  }

  if (code === 0x09) {
    return { name: "tab", ctrl: false, shift: false, sequence: seq };
  }

  if (code >= 1 && code <= 26) {
    const name = CTRL_KEYS[code] ?? String.fromCharCode(code + 96);
    return { name, ctrl: true, shift: false, sequence: seq };
  }

  if (code >= 32 && code < 127) {
    return { name: seq, ctrl: false, shift: false, sequence: seq };
  }

  return { name: "unknown", ctrl: false, shift: false, sequence: seq };
};
