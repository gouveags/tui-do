import { cursor, screen, style } from "./ansi.ts";
import { createBuffer } from "./buffer.ts";

export type TerminalSize = {
  rows: number;
  cols: number;
};

export const getTerminalSize = (): TerminalSize => ({
  rows: process.stdout.rows || 24,
  cols: process.stdout.columns || 80,
});

export type Screen = {
  writeAt: (row: number, col: number, text: string) => void;
  writeStyled: (row: number, col: number, styles: string, text: string) => void;
  flush: () => string;
  prepare: () => string;
  restore: () => string;
};

export const createScreen = (bufferSize = 8192): Screen => {
  const buf = createBuffer(bufferSize);

  return {
    writeAt(row, col, text) {
      buf.write(cursor.moveTo(row, col));
      buf.write(text);
    },
    writeStyled(row, col, styles, text) {
      buf.write(cursor.moveTo(row, col));
      buf.write(styles);
      buf.write(text);
      buf.write(style.reset);
    },
    flush() {
      const output = buf.toString();
      buf.clear();
      return output;
    },
    prepare() {
      return cursor.hide + cursor.home + screen.clear;
    },
    restore() {
      return cursor.show;
    },
  };
};
