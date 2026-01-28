import { cursor, screen as screenCodes } from "./ansi.ts";
import { diffGrids } from "./diff.ts";
import { clearGrid, createGrid, setCell } from "./grid.ts";

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
  setSize: (size: TerminalSize) => void;
};

const splitText = (text: string): string[] => [...text];

export const createScreen = (
  initialSize: TerminalSize = getTerminalSize(),
): Screen => {
  let size = initialSize;
  let prevGrid = createGrid(size.cols, size.rows);
  let nextGrid = createGrid(size.cols, size.rows);
  let needsFullClear = true;

  return {
    writeAt(row, col, text) {
      let currentCol = col;
      for (const ch of splitText(text)) {
        setCell(nextGrid, row - 1, currentCol - 1, { ch, style: "" });
        currentCol += 1;
      }
    },
    writeStyled(row, col, styles, text) {
      let currentCol = col;
      for (const ch of splitText(text)) {
        setCell(nextGrid, row - 1, currentCol - 1, { ch, style: styles });
        currentCol += 1;
      }
    },
    flush() {
      let output = "";
      if (needsFullClear) {
        output += screenCodes.clear + cursor.home;
        needsFullClear = false;
      }
      output += diffGrids(prevGrid, nextGrid);
      const previous = prevGrid;
      prevGrid = nextGrid;
      nextGrid = previous;
      return output;
    },
    prepare() {
      clearGrid(nextGrid);
      return cursor.hide;
    },
    restore() {
      return cursor.show;
    },
    setSize(newSize) {
      if (newSize.cols === size.cols && newSize.rows === size.rows) {
        return;
      }
      size = newSize;
      prevGrid = createGrid(size.cols, size.rows);
      nextGrid = createGrid(size.cols, size.rows);
      needsFullClear = true;
    },
  };
};
