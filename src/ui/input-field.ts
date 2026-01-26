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

const drawBoxLine = (
  screen: Screen,
  row: number,
  col: number,
  width: number,
  content: string,
  borderColor: string,
): void => {
  const innerWidth = width - 2;
  const paddedContent = content.padEnd(innerWidth).slice(0, innerWidth);
  screen.writeStyled(row, col, borderColor, BOX_CHARS.vertical);
  screen.writeAt(row, col + 1, paddedContent);
  screen.writeStyled(
    row,
    col + 1 + innerWidth,
    borderColor,
    BOX_CHARS.vertical,
  );
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
  const innerWidth = width - 2;
  const textPadding = padding - 1;
  const labelLen = label ? label.length + 1 : 0;
  const textAreaWidth = innerWidth - textPadding * 2 - labelLen;

  if (withBorders) {
    screen.writeStyled(
      currentRow,
      col,
      theme.colors.inputBorder,
      BOX_CHARS.roundedTopLeft +
        BOX_CHARS.horizontal.repeat(innerWidth) +
        BOX_CHARS.roundedTopRight,
    );
    currentRow++;
  }

  drawBoxLine(screen, currentRow, col, width, "", theme.colors.inputBorder);
  currentRow++;

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

    const isCursorLine = lineIndex === cursor.line;
    const cursorCol = cursor.col;

    const scrollOffset = Math.max(0, cursorCol - textAreaWidth + 1);
    const visibleText = line.slice(scrollOffset, scrollOffset + textAreaWidth);
    const cursorPosInView = cursorCol - scrollOffset;

    screen.writeStyled(
      displayRow,
      col,
      theme.colors.inputBorder,
      BOX_CHARS.vertical,
    );
    screen.writeAt(displayRow, col + 1, " ".repeat(textPadding));

    let textStartCol = col + 1 + textPadding;

    if (label && i === 0) {
      screen.writeStyled(
        displayRow,
        textStartCol,
        theme.colors.inputLabel,
        label,
      );
      screen.writeAt(displayRow, textStartCol + label.length, " ");
      textStartCol += labelLen;
    } else if (label) {
      screen.writeAt(displayRow, textStartCol, " ".repeat(labelLen));
      textStartCol += labelLen;
    }

    if (
      showCursor &&
      isCursorLine &&
      cursorPosInView >= 0 &&
      cursorPosInView < textAreaWidth
    ) {
      const before = visibleText.slice(0, cursorPosInView);
      const cursorChar = visibleText[cursorPosInView] ?? " ";
      const after = visibleText.slice(cursorPosInView + 1);

      screen.writeAt(displayRow, textStartCol, before);
      screen.writeStyled(
        displayRow,
        textStartCol + before.length,
        style.inverse,
        cursorChar,
      );
      screen.writeAt(
        displayRow,
        textStartCol + before.length + 1,
        after.padEnd(textAreaWidth - before.length - 1),
      );
    } else {
      screen.writeAt(
        displayRow,
        textStartCol,
        visibleText.padEnd(textAreaWidth),
      );
    }

    screen.writeAt(
      displayRow,
      textStartCol + textAreaWidth,
      " ".repeat(textPadding),
    );
    screen.writeStyled(
      displayRow,
      col + 1 + innerWidth,
      theme.colors.inputBorder,
      BOX_CHARS.vertical,
    );
  }

  currentRow += visibleLines;

  drawBoxLine(screen, currentRow, col, width, "", theme.colors.inputBorder);
  currentRow++;

  if (withBorders) {
    screen.writeStyled(
      currentRow,
      col,
      theme.colors.inputBorder,
      BOX_CHARS.roundedBottomLeft +
        BOX_CHARS.horizontal.repeat(innerWidth) +
        BOX_CHARS.roundedBottomRight,
    );
  }
};

export const getInputFieldHeight = (
  input: InputBuffer,
  maxLines: number,
  withBorders = true,
): number => {
  const contentLines = Math.min(input.lines.length, maxLines);
  const borderLines = withBorders ? 2 : 0;
  const emptyLines = 2;
  return contentLines + borderLines + emptyLines;
};
