import { cursor, screen as screenCodes } from "./ansi.ts";
import { diffGrids } from "./diff.ts";
import { clearGrid, createGrid, getCell, setCell } from "./grid.ts";
import { charWidth } from "./width.ts";

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

const splitText = (text: string): string[] => Array.from(text);
const PLACEHOLDER_CHAR = "\u0000";

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
      let lastCellCol = col - 1;
      for (const ch of splitText(text)) {
        const width = charWidth(ch);
        if (width === 0) {
          if (lastCellCol >= 0) {
            const cell = getCell(nextGrid, row - 1, lastCellCol);
            cell.ch += ch;
          }
          continue;
        }
        setCell(nextGrid, row - 1, currentCol - 1, { ch, style: "" });
        lastCellCol = currentCol - 1;
        if (width === 2) {
          setCell(nextGrid, row - 1, currentCol, {
            ch: PLACEHOLDER_CHAR,
            style: "",
          });
        }
        currentCol += width;
      }
    },
    writeStyled(row, col, styles, text) {
      let currentCol = col;
      let lastCellCol = col - 1;
      for (const ch of splitText(text)) {
        const width = charWidth(ch);
        if (width === 0) {
          if (lastCellCol >= 0) {
            const cell = getCell(nextGrid, row - 1, lastCellCol);
            cell.ch += ch;
          }
          continue;
        }
        setCell(nextGrid, row - 1, currentCol - 1, { ch, style: styles });
        lastCellCol = currentCol - 1;
        if (width === 2) {
          setCell(nextGrid, row - 1, currentCol, {
            ch: PLACEHOLDER_CHAR,
            style: "",
          });
        }
        currentCol += width;
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
