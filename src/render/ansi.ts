export const ANSI = {
  ESC: "\x1b",
  CSI: "\x1b[",
} as const;

export const cursor = {
  hide: "\x1b[?25l",
  show: "\x1b[?25h",
  home: "\x1b[H",
  moveTo: (row: number, col: number) => `\x1b[${row};${col}H`,
} as const;

export const style = {
  reset: "\x1b[0m",
  bold: "\x1b[1m",
  dim: "\x1b[2m",
  underline: "\x1b[4m",
  inverse: "\x1b[7m",
  fg: {
    black: "\x1b[30m",
    red: "\x1b[31m",
    green: "\x1b[32m",
    yellow: "\x1b[33m",
    blue: "\x1b[34m",
    magenta: "\x1b[35m",
    cyan: "\x1b[36m",
    white: "\x1b[37m",
  },
  bg: {
    black: "\x1b[40m",
    red: "\x1b[41m",
    green: "\x1b[42m",
    yellow: "\x1b[43m",
    blue: "\x1b[44m",
    magenta: "\x1b[45m",
    cyan: "\x1b[46m",
    white: "\x1b[47m",
  },
} as const;

export const screen = {
  clear: "\x1b[2J",
  clearLine: "\x1b[2K",
  altBuffer: "\x1b[?1049h",
  mainBuffer: "\x1b[?1049l",
} as const;

export const sync = {
  begin: "\x1b[?2026h",
  end: "\x1b[?2026l",
} as const;

export const keyboard = {
  enableKittyProtocol: "\x1b[>1u",
  disableKittyProtocol: "\x1b[<u",
} as const;

export const fg256 = (code: number) => `\x1b[38;5;${code}m`;
export const bg256 = (code: number) => `\x1b[48;5;${code}m`;
