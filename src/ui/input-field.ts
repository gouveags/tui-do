import { style } from "../render/ansi.ts";
import type { Screen } from "../render/screen.ts";
import { defaultTheme, type Theme } from "../render/theme.ts";
import type { InputBuffer } from "../state/types.ts";
import { BOX_CHARS } from "./components.ts";

export type InputFieldOptions = {
  width: number;
  maxLines?: number;
  showCursor?: boolean;
  label?: string;
  withBorders?: boolean;
  padding?: number;
};

export const drawInputField = (
  screen: Screen,
  row: number,
  col: number,
  input: InputBuffer,
  options: InputFieldOptions,
  theme: Theme = defaultTheme,
): void => {
  const {
    width,
    maxLines = 1,
    showCursor = true,
    label,
    withBorders = true,
    padding = 2,
  } = options;

  let currentRow = row;
  const innerWidth = width - padding * 2;

  if (withBorders) {
    screen.writeStyled(
      currentRow,
      col,
      theme.colors.border,
      BOX_CHARS.horizontal.repeat(width),
    );
    currentRow++;
  }

  screen.writeAt(currentRow, col, " ".repeat(width));
  currentRow++;

  if (label) {
    screen.writeAt(currentRow, col, " ".repeat(width));
    screen.writeStyled(currentRow, col + padding, theme.colors.help, label);
    currentRow++;
  }

  screen.writeAt(currentRow, col, " ".repeat(width));

  const { lines, cursor } = input;
  const visibleLines = maxLines === 1 ? 1 : Math.min(lines.length, maxLines);
  const startLine = Math.max(
    0,
    cursor.line - maxLines + 1,
    lines.length - maxLines,
  );

  for (let i = 0; i < visibleLines; i++) {
    const lineIndex = startLine + i;
    const line = lines[lineIndex] ?? "";
    const displayRow = currentRow + i;
    const displayCol = col + padding;

    const isCursorLine = lineIndex === cursor.line;
    const cursorCol = cursor.col;

    screen.writeAt(displayRow, col, " ".repeat(width));

    const scrollOffset = Math.max(0, cursorCol - innerWidth + 1);
    const visibleText = line.slice(scrollOffset, scrollOffset + innerWidth);
    const cursorPosInView = cursorCol - scrollOffset;

    if (
      showCursor &&
      isCursorLine &&
      cursorPosInView >= 0 &&
      cursorPosInView < innerWidth
    ) {
      const before = visibleText.slice(0, cursorPosInView);
      const cursorChar = visibleText[cursorPosInView] ?? " ";
      const after = visibleText.slice(cursorPosInView + 1);

      screen.writeAt(displayRow, displayCol, before);
      screen.writeStyled(
        displayRow,
        displayCol + before.length,
        style.inverse,
        cursorChar,
      );
      screen.writeAt(
        displayRow,
        displayCol + before.length + 1,
        after.padEnd(innerWidth - before.length - 1),
      );
    } else {
      screen.writeAt(displayRow, displayCol, visibleText.padEnd(innerWidth));
    }
  }

  currentRow += visibleLines;

  screen.writeAt(currentRow, col, " ".repeat(width));
  currentRow++;

  if (withBorders) {
    screen.writeStyled(
      currentRow,
      col,
      theme.colors.border,
      BOX_CHARS.horizontal.repeat(width),
    );
  }
};

export const getInputFieldHeight = (
  input: InputBuffer,
  maxLines: number,
  withBorders = true,
  hasLabel = false,
): number => {
  const contentLines = Math.min(input.lines.length, maxLines);
  const borderLines = withBorders ? 2 : 0;
  const labelLine = hasLabel ? 1 : 0;
  const paddingLines = 2;
  return contentLines + borderLines + labelLine + paddingLines;
};
