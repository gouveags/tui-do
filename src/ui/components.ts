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
  roundedTopLeft: "╭",
  roundedTopRight: "╮",
  roundedBottomLeft: "╰",
  roundedBottomRight: "╯",
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
  const indicator = selected ? "▸" : "•";
  const content = ` ${indicator} ${text}`.slice(0, width);
  const paddedText = content.padEnd(width);

  if (selected) {
    drawHighlightedText(screen, row, col, paddedText, true, theme);
    return;
  }

  screen.writeAt(row, col, paddedText);
  screen.writeStyled(row, col + 1, theme.colors.bullet, indicator);
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

export const drawStatusBar = (
  screen: Screen,
  row: number,
  width: number,
  leftText: string,
  rightText: string,
  theme: Theme = defaultTheme,
): void => {
  screen.writeStyled(row, 1, theme.colors.statusBar, " ".repeat(width));

  const left = leftText.slice(0, width);
  screen.writeStyled(row, 2, theme.colors.statusBarAccent, left);

  if (rightText.length > 0) {
    const rightStart = Math.max(1, width - rightText.length + 1);
    if (rightStart > 1) {
      screen.writeStyled(
        row,
        rightStart,
        theme.colors.statusBarAccent,
        rightText.slice(0, width - rightStart + 1),
      );
    }
  }
};

export const drawSectionHeader = (
  screen: Screen,
  row: number,
  width: number,
  title: string,
  subtitle?: string,
  theme: Theme = defaultTheme,
): number => {
  screen.writeStyled(
    row,
    1,
    theme.colors.panelBorder,
    BOX_CHARS.horizontal.repeat(width),
  );

  screen.writeAt(row + 1, 1, " ".repeat(width));
  screen.writeStyled(row + 1, 3, theme.colors.panelTitle, title);

  let nextRow = row + 2;
  if (subtitle) {
    screen.writeAt(nextRow, 1, " ".repeat(width));
    screen.writeStyled(nextRow, 3, theme.colors.help, subtitle);
    nextRow += 1;
  }

  screen.writeStyled(
    nextRow,
    1,
    theme.colors.panelBorder,
    BOX_CHARS.horizontal.repeat(width),
  );
  return nextRow - row + 1;
};

export const drawHintLine = (
  screen: Screen,
  row: number,
  col: number,
  text: string,
  theme: Theme = defaultTheme,
): void => {
  const regex = /\[[^\]]+]/g;
  let lastIndex = 0;
  let cursor = col;
  let match = regex.exec(text);

  while (match !== null) {
    const before = text.slice(lastIndex, match.index);
    if (before) {
      screen.writeStyled(row, cursor, theme.colors.hintText, before);
      cursor += before.length;
    }

    const key = match[0];
    screen.writeStyled(row, cursor, theme.colors.hintKey, key);
    cursor += key.length;
    lastIndex = match.index + key.length;
    match = regex.exec(text);
  }

  const remaining = text.slice(lastIndex);
  if (remaining) {
    screen.writeStyled(row, cursor, theme.colors.hintText, remaining);
  }
};
