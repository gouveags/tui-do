export type KeyEvent = {
  name: string;
  ctrl: boolean;
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
};

export const parseKey = (buf: Buffer): KeyEvent => {
  const seq = buf.toString();
  const code = buf[0];

  if (code === undefined) {
    return { name: "unknown", ctrl: false, sequence: seq };
  }

  if (code === 0x1b) {
    if (buf.length === 1) {
      return { name: "escape", ctrl: false, sequence: seq };
    }
    const suffix = seq.slice(1);
    const name = ESCAPE_SEQUENCES[suffix];
    return { name: name ?? "unknown", ctrl: false, sequence: seq };
  }

  if (code === 0x0d) {
    return { name: "enter", ctrl: false, sequence: seq };
  }

  if (code === 0x7f) {
    return { name: "backspace", ctrl: false, sequence: seq };
  }

  if (code === 0x09) {
    return { name: "tab", ctrl: false, sequence: seq };
  }

  if (code >= 1 && code <= 26) {
    const name = CTRL_KEYS[code] ?? String.fromCharCode(code + 96);
    return { name, ctrl: true, sequence: seq };
  }

  if (code >= 32 && code < 127) {
    return { name: seq, ctrl: false, sequence: seq };
  }

  return { name: "unknown", ctrl: false, sequence: seq };
};
