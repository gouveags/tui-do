import {
  disableRawMode,
  enableRawMode,
  startKeyListener,
} from "./input/keyboard.ts";
import type { KeyEvent } from "./input/keys.ts";
import { cursor, screen as screenCodes } from "./render/ansi.ts";
import { getTerminalSize } from "./render/layout.ts";
import { createScreen } from "./render/screen.ts";
import type { Action } from "./state/app.ts";
import { transition } from "./state/app.ts";
import type { AppState } from "./state/types.ts";
import { createInitialState } from "./state/types.ts";
import { ensureStorageDir, listTodos, saveTodo } from "./storage/todos.ts";
import { render } from "./ui/menu.ts";

const keyToAction = (key: KeyEvent, state: AppState): Action | null => {
  if (key.ctrl && key.name === "c") return { type: "QUIT" };
  if (key.name === "escape") return { type: "BACK" };

  if (state.view === "main_menu") {
    switch (key.name) {
      case "up":
        return { type: "NAVIGATE_UP" };
      case "down":
        return { type: "NAVIGATE_DOWN" };
      case "enter":
        return { type: "SELECT" };
      case "1":
        return { type: "QUICK_SELECT", option: 1 };
      case "2":
        return { type: "QUICK_SELECT", option: 2 };
      case "3":
        return { type: "QUICK_SELECT", option: 3 };
    }
    return null;
  }

  if (state.view === "load_todo") {
    switch (key.name) {
      case "up":
        return { type: "NAVIGATE_UP" };
      case "down":
        return { type: "NAVIGATE_DOWN" };
      case "enter":
        return { type: "SELECT" };
    }
    return null;
  }

  if (state.view === "create_todo" || state.view === "view_todo") {
    switch (key.name) {
      case "enter":
        return { type: "SUBMIT" };
      case "backspace":
        return { type: "INPUT_BACKSPACE" };
      case "delete":
        return { type: "DELETE_FORWARD" };
      case "left":
        return { type: "CURSOR_LEFT" };
      case "right":
        return { type: "CURSOR_RIGHT" };
      case "home":
        return { type: "CURSOR_HOME" };
      case "end":
        return { type: "CURSOR_END" };
    }

    if (key.name.length === 1 && !key.ctrl) {
      return { type: "INPUT_CHAR", char: key.name };
    }
  }

  if (state.view === "view_todo") {
    switch (key.name) {
      case "up":
        return { type: "NAVIGATE_UP" };
      case "down":
        return { type: "NAVIGATE_DOWN" };
    }
  }

  return null;
};

export const runApp = (): void => {
  ensureStorageDir();
  const todos = listTodos();
  const terminalSize = getTerminalSize();
  let state: AppState = { ...createInitialState(), todos, terminalSize };
  const scr = createScreen();
  let stopListener: (() => void) | null = null;

  const renderFrame = () => {
    process.stdout.write(scr.prepare());
    render(scr, state);
    process.stdout.write(scr.flush());
  };

  const cleanup = () => {
    process.stdout.write(screenCodes.clear + cursor.home + cursor.show);
    process.removeListener("SIGWINCH", handleResize);
    disableRawMode();
    if (stopListener) stopListener();
  };

  const handleResize = () => {
    const newSize = getTerminalSize();
    state = transition(state, {
      type: "RESIZE",
      rows: newSize.rows,
      cols: newSize.cols,
    });
    renderFrame();
  };

  const handleKey = (key: KeyEvent) => {
    const action = keyToAction(key, state);
    if (!action) return;

    const prevView = state.view;
    state = transition(state, action);

    if (state.view === "quit") {
      cleanup();
      process.exit(0);
    }

    if (state.view === "main_menu" && prevView !== "main_menu") {
      state = { ...state, todos: listTodos() };
    }

    const selectedTodo = state.todos.find((t) => t.id === state.selectedTodoId);
    if (
      selectedTodo &&
      (action.type === "SUBMIT" || action.type === "SELECT")
    ) {
      saveTodo(selectedTodo);
    }

    renderFrame();
  };

  process.on("SIGWINCH", handleResize);
  enableRawMode();
  stopListener = startKeyListener(handleKey);
  renderFrame();
};
