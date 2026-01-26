import type { Screen } from "../render/screen.ts";
import { defaultTheme, type Theme } from "../render/theme.ts";

export const BOX_CHARS = {
  topLeft: "┌",
  topRight: "┐",
  bottomLeft: "└",
  bottomRight: "┘",
  horizontal: "─",
  vertical: "│",
  leftT: "├",
  rightT: "┤",
} as const;

export const drawFullWidthLine = (
  screen: Screen,
  row: number,
  width: number,
  theme: Theme = defaultTheme,
): void => {
  screen.writeStyled(
    row,
    1,
    theme.colors.border,
    BOX_CHARS.horizontal.repeat(width),
  );
};

export const drawFullWidthBox = (
  screen: Screen,
  startRow: number,
  width: number,
  height: number,
  title?: string,
  theme: Theme = defaultTheme,
): void => {
  const titlePart = title ? ` ${title} ` : "";
  const titleStyled = title
    ? ` ${theme.colors.title}${title}${theme.reset} `
    : "";
  const borderLen = Math.max(0, width - 2 - titlePart.length);

  const topLine =
    BOX_CHARS.horizontal + titleStyled + BOX_CHARS.horizontal.repeat(borderLen);

  screen.writeStyled(startRow, 1, theme.colors.border, topLine);

  for (let i = 1; i < height - 1; i++) {
    screen.writeAt(startRow + i, 1, " ".repeat(width));
  }

  screen.writeStyled(
    startRow + height - 1,
    1,
    theme.colors.border,
    BOX_CHARS.horizontal.repeat(width),
  );
};

export const drawBox = (
  screen: Screen,
  row: number,
  col: number,
  width: number,
  height: number,
  title?: string,
  theme: Theme = defaultTheme,
): void => {
  const titlePart = title ? ` ${title} ` : "";
  const titleStyled = title
    ? ` ${theme.colors.title}${title}${theme.reset} `
    : "";
  const borderLen = Math.max(0, width - 4 - titlePart.length);

  const topLine =
    BOX_CHARS.topLeft +
    BOX_CHARS.horizontal +
    titleStyled +
    BOX_CHARS.horizontal.repeat(borderLen) +
    BOX_CHARS.topRight;

  screen.writeStyled(row, col, theme.colors.border, topLine);

  for (let i = 1; i < height - 1; i++) {
    screen.writeStyled(
      row + i,
      col,
      theme.colors.border,
      BOX_CHARS.vertical + " ".repeat(width - 2) + BOX_CHARS.vertical,
    );
  }

  screen.writeStyled(
    row + height - 1,
    col,
    theme.colors.border,
    BOX_CHARS.bottomLeft +
      BOX_CHARS.horizontal.repeat(width - 2) +
      BOX_CHARS.bottomRight,
  );
};

export const drawText = (
  screen: Screen,
  row: number,
  col: number,
  text: string,
  maxWidth?: number,
): void => {
  const displayText = maxWidth ? text.slice(0, maxWidth) : text;
  screen.writeAt(row, col, displayText);
};

export const drawHighlightedText = (
  screen: Screen,
  row: number,
  col: number,
  text: string,
  highlighted: boolean,
  theme: Theme = defaultTheme,
): void => {
  if (highlighted) {
    screen.writeStyled(row, col, theme.colors.selected, text);
  } else {
    screen.writeAt(row, col, text);
  }
};

export const drawMenuItem = (
  screen: Screen,
  row: number,
  col: number,
  text: string,
  selected: boolean,
  width: number,
  theme: Theme = defaultTheme,
): void => {
  const prefix = selected ? "  ▸ " : "    ";
  const paddedText = (prefix + text).padEnd(width);
  drawHighlightedText(screen, row, col, paddedText, selected, theme);
};

export const drawInput = (
  screen: Screen,
  row: number,
  col: number,
  label: string,
  value: string,
  width: number,
  theme: Theme = defaultTheme,
): void => {
  screen.writeAt(row, col, label);
  const inputWidth = width - label.length - 2;
  const displayValue = value.slice(-inputWidth).padEnd(inputWidth);
  screen.writeStyled(
    row,
    col + label.length,
    theme.colors.help,
    `[${displayValue}]`,
  );
};

export const drawHelp = (
  screen: Screen,
  row: number,
  col: number,
  text: string,
  theme: Theme = defaultTheme,
): void => {
  screen.writeStyled(row, col, theme.colors.help, text);
};

export const drawCompletedItem = (
  screen: Screen,
  row: number,
  col: number,
  text: string,
  theme: Theme = defaultTheme,
): void => {
  screen.writeStyled(row, col, theme.colors.completed, text);
};

export const drawError = (
  screen: Screen,
  row: number,
  col: number,
  text: string,
  theme: Theme = defaultTheme,
): void => {
  screen.writeStyled(row, col, theme.colors.error, text);
};

export const drawSeparator = (
  screen: Screen,
  row: number,
  col: number,
  width: number,
  theme: Theme = defaultTheme,
): void => {
  screen.writeStyled(
    row,
    col,
    theme.colors.border,
    BOX_CHARS.horizontal.repeat(width),
  );
};
