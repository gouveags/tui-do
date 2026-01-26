import { renderMarkdownLines } from "../markdown/renderer.ts";
import { renderAsciiArt } from "../render/ascii-art.ts";
import type { Screen } from "../render/screen.ts";
import { defaultTheme } from "../render/theme.ts";
import type { AppState } from "../state/types.ts";
import {
  BOX_CHARS,
  drawCompletedItem,
  drawHelp,
  drawHighlightedText,
  drawMenuItem,
  drawText,
} from "./components.ts";
import { drawInputField } from "./input-field.ts";

const MAIN_MENU_ITEMS = ["Create a new to-do", "Load a to-do", "Quit"];

const PADDING = 4;
const HEADER_HEIGHT = 7;

const drawHeader = (
  screen: Screen,
  row: number,
  width: number,
  title: string,
  useAsciiArt = true,
): number => {
  screen.writeStyled(
    row,
    1,
    defaultTheme.colors.border,
    BOX_CHARS.horizontal.repeat(width),
  );

  if (useAsciiArt) {
    const artLines = renderAsciiArt(title);
    screen.writeAt(row + 1, 1, " ".repeat(width));
    for (let i = 0; i < artLines.length; i++) {
      screen.writeAt(row + 2 + i, 1, " ".repeat(width));
      screen.writeStyled(
        row + 2 + i,
        PADDING,
        defaultTheme.colors.title,
        artLines[i] ?? "",
      );
    }
    screen.writeAt(row + 5, 1, " ".repeat(width));
    screen.writeStyled(
      row + 6,
      1,
      defaultTheme.colors.border,
      BOX_CHARS.horizontal.repeat(width),
    );
    return HEADER_HEIGHT;
  }

  screen.writeAt(row + 1, 1, " ".repeat(width));
  screen.writeStyled(row + 2, PADDING, defaultTheme.colors.title, title);
  screen.writeAt(row + 3, 1, " ".repeat(width));
  screen.writeStyled(
    row + 4,
    1,
    defaultTheme.colors.border,
    BOX_CHARS.horizontal.repeat(width),
  );
  return 5;
};

const drawFooter = (
  screen: Screen,
  row: number,
  width: number,
  helpTexts: string[],
): void => {
  screen.writeStyled(
    row,
    1,
    defaultTheme.colors.border,
    BOX_CHARS.horizontal.repeat(width),
  );

  screen.writeAt(row + 1, 1, " ".repeat(width));

  const helpLine = helpTexts.join("   ");
  screen.writeStyled(row + 2, PADDING, defaultTheme.colors.help, helpLine);
};

export const renderMainMenu = (screen: Screen, state: AppState): void => {
  const width = state.terminalSize.cols;
  let currentRow = 1;

  currentRow += drawHeader(screen, currentRow, width, "To-Do");

  screen.writeAt(currentRow, 1, " ".repeat(width));
  currentRow++;

  for (let i = 0; i < MAIN_MENU_ITEMS.length; i++) {
    const item = MAIN_MENU_ITEMS[i];
    if (item) {
      screen.writeAt(currentRow, 1, " ".repeat(width));
      drawMenuItem(
        screen,
        currentRow,
        PADDING,
        `${i + 1}. ${item}`,
        i === state.menuIndex,
        width - PADDING * 2,
      );
      currentRow++;

      screen.writeAt(currentRow, 1, " ".repeat(width));
      currentRow++;
    }
  }

  currentRow++;

  drawFooter(screen, currentRow, width, [
    "[↑/↓] Navigate",
    "[Enter] Select",
    "[1-3] Quick select",
    "[Ctrl+C] Quit",
  ]);
};

export const renderCreateTodo = (screen: Screen, state: AppState): void => {
  const width = state.terminalSize.cols;
  const inputWidth = Math.min(60, width - PADDING * 2);
  let currentRow = 1;

  currentRow += drawHeader(screen, currentRow, width, "Create");

  screen.writeAt(currentRow, 1, " ".repeat(width));
  currentRow++;

  screen.writeAt(currentRow, 1, " ".repeat(width));
  drawText(screen, currentRow, PADDING, "Enter a name for your to-do:");
  currentRow += 2;

  drawInputField(screen, currentRow, PADDING, state.input, {
    width: inputWidth,
    label: "Name:",
    showCursor: true,
    withBorders: true,
    padding: PADDING,
  });
  currentRow += 5;

  drawFooter(screen, currentRow, width, [
    "[Enter] Create",
    "[ESC] Cancel",
    "[←/→] Move cursor",
  ]);
};

export const renderLoadTodo = (screen: Screen, state: AppState): void => {
  const width = state.terminalSize.cols;
  const todoCount = state.todos.length;
  let currentRow = 1;

  currentRow += drawHeader(screen, currentRow, width, "Load");

  screen.writeAt(currentRow, 1, " ".repeat(width));
  currentRow++;

  if (todoCount === 0) {
    screen.writeAt(currentRow, 1, " ".repeat(width));
    drawHelp(screen, currentRow, PADDING, "No to-dos found.");
    currentRow += 3;
  } else {
    const maxVisible = Math.min(todoCount, state.terminalSize.rows - 16);
    const startIdx = Math.max(0, state.menuIndex - maxVisible + 1);

    for (let i = 0; i < maxVisible && startIdx + i < todoCount; i++) {
      const todo = state.todos[startIdx + i];
      if (todo) {
        const doneCount = todo.items.filter((it) => it.done).length;
        const label = `${todo.title} (${doneCount}/${todo.items.length})`;

        screen.writeAt(currentRow, 1, " ".repeat(width));
        drawMenuItem(
          screen,
          currentRow,
          PADDING,
          label,
          startIdx + i === state.menuIndex,
          width - PADDING * 2,
        );
        currentRow++;

        screen.writeAt(currentRow, 1, " ".repeat(width));
        currentRow++;
      }
    }
  }

  currentRow++;

  drawFooter(screen, currentRow, width, [
    "[↑/↓] Navigate",
    "[Enter] Select",
    "[ESC] Back",
  ]);
};

export const renderViewTodo = (screen: Screen, state: AppState): void => {
  const todo = state.todos.find((t) => t.id === state.selectedTodoId);
  if (!todo) return;

  const width = state.terminalSize.cols;
  const itemCount = todo.items.length;
  let currentRow = 1;

  const shortTitle =
    todo.title.length > 12 ? todo.title.slice(0, 12) : todo.title;
  currentRow += drawHeader(screen, currentRow, width, shortTitle);

  screen.writeAt(currentRow, 1, " ".repeat(width));
  currentRow++;

  if (itemCount === 0) {
    screen.writeAt(currentRow, 1, " ".repeat(width));
    drawHelp(screen, currentRow, PADDING, "No items yet. Add one below.");
    currentRow += 2;
  } else {
    const maxVisible = Math.min(itemCount, state.terminalSize.rows - 20);
    const startIdx = Math.max(0, state.menuIndex - maxVisible + 1);

    for (let i = 0; i < maxVisible && startIdx + i < itemCount; i++) {
      const item = todo.items[startIdx + i];
      if (item) {
        const checkbox = item.done ? "[✓]" : "[ ]";
        const isSelected = startIdx + i === state.menuIndex;
        const textWidth = width - PADDING * 2 - 6;

        const renderedLines = renderMarkdownLines(item.text, textWidth);
        const displayText = renderedLines[0] ?? item.text;
        const fullText = `${checkbox}  ${displayText}`.slice(
          0,
          width - PADDING * 2,
        );

        screen.writeAt(currentRow, 1, " ".repeat(width));

        if (item.done && !isSelected) {
          drawCompletedItem(
            screen,
            currentRow,
            PADDING,
            fullText.padEnd(width - PADDING * 2),
          );
        } else {
          drawHighlightedText(
            screen,
            currentRow,
            PADDING,
            fullText.padEnd(width - PADDING * 2),
            isSelected,
          );
        }
        currentRow++;

        screen.writeAt(currentRow, 1, " ".repeat(width));
        currentRow++;
      }
    }
  }

  currentRow++;

  const inputWidth = Math.min(60, width - PADDING * 2);
  drawInputField(screen, currentRow, PADDING, state.input, {
    width: inputWidth,
    label: "Add item:",
    showCursor: true,
    withBorders: true,
    padding: PADDING,
  });
  currentRow += 5;

  drawFooter(screen, currentRow, width, [
    "[↑/↓] Navigate",
    "[Enter] Toggle/Add",
    "[←/→] Move cursor",
    "[ESC] Back",
  ]);
};

export const render = (screen: Screen, state: AppState): void => {
  switch (state.view) {
    case "main_menu":
      renderMainMenu(screen, state);
      break;
    case "create_todo":
      renderCreateTodo(screen, state);
      break;
    case "load_todo":
      renderLoadTodo(screen, state);
      break;
    case "view_todo":
      renderViewTodo(screen, state);
      break;
  }
};
