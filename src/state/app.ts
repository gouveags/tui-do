import {
  clearBuffer,
  deleteBackward,
  deleteForward,
  getBufferText,
  insertChar,
  insertNewline,
  moveDown,
  moveLeft,
  moveRight,
  moveToLineEnd,
  moveToLineStart,
  moveUp,
} from "../input/cursor.ts";
import { filterByDate, filterTodos, nextDateFilter } from "../utils/search.ts";
import type { AppState, InputBuffer, Todo, TodoItem } from "./types.ts";

export type Action =
  | { type: "NAVIGATE_UP" }
  | { type: "NAVIGATE_DOWN" }
  | { type: "SELECT" }
  | { type: "BACK" }
  | { type: "QUIT" }
  | { type: "QUICK_SELECT"; option: number }
  | { type: "INPUT_CHAR"; char: string }
  | { type: "INPUT_BACKSPACE" }
  | { type: "SUBMIT" }
  | { type: "LOAD_TODOS"; todos: Todo[] }
  | { type: "CURSOR_LEFT" }
  | { type: "CURSOR_RIGHT" }
  | { type: "CURSOR_UP" }
  | { type: "CURSOR_DOWN" }
  | { type: "CURSOR_HOME" }
  | { type: "CURSOR_END" }
  | { type: "INSERT_NEWLINE" }
  | { type: "DELETE_FORWARD" }
  | { type: "RESIZE"; rows: number; cols: number }
  | { type: "MOVE_ITEM_UP" }
  | { type: "MOVE_ITEM_DOWN" }
  | { type: "ENTER_SEARCH" }
  | { type: "TOGGLE_DATE_FILTER" };

const MAIN_MENU_OPTIONS = 3;

const generateId = (): string =>
  Date.now().toString(36) + Math.random().toString(36).slice(2, 7);

const syncInputBuffer = (state: AppState): AppState => ({
  ...state,
  inputBuffer: getBufferText(state.input),
});

const withInput = (state: AppState, input: InputBuffer): AppState =>
  syncInputBuffer({ ...state, input });

const mainMenuTransition = (state: AppState, action: Action): AppState => {
  switch (action.type) {
    case "NAVIGATE_DOWN":
      return { ...state, menuIndex: (state.menuIndex + 1) % MAIN_MENU_OPTIONS };
    case "NAVIGATE_UP":
      return {
        ...state,
        menuIndex:
          (state.menuIndex - 1 + MAIN_MENU_OPTIONS) % MAIN_MENU_OPTIONS,
      };
    case "SELECT": {
      const views = ["create_todo", "load_todo", "quit"] as const;
      const nextView = views[state.menuIndex] ?? "main_menu";
      return {
        ...state,
        view: nextView,
        menuIndex: 0,
        inputMode: nextView === "create_todo" ? "multi" : "single",
      };
    }
    case "QUICK_SELECT": {
      const views = ["create_todo", "load_todo", "quit"] as const;
      const nextView = views[action.option - 1] ?? "main_menu";
      return {
        ...state,
        view: nextView,
        menuIndex: 0,
        inputMode: nextView === "create_todo" ? "multi" : "single",
      };
    }
    default:
      return state;
  }
};

const handleInputAction = (
  state: AppState,
  action: Action,
): AppState | null => {
  switch (action.type) {
    case "INPUT_CHAR":
      return withInput(state, insertChar(state.input, action.char));
    case "INPUT_BACKSPACE":
      return withInput(state, deleteBackward(state.input));
    case "DELETE_FORWARD":
      return withInput(state, deleteForward(state.input));
    case "CURSOR_LEFT":
      return withInput(state, moveLeft(state.input));
    case "CURSOR_RIGHT":
      return withInput(state, moveRight(state.input));
    case "CURSOR_UP":
      if (state.inputMode === "multi") {
        return withInput(state, moveUp(state.input));
      }
      return null;
    case "CURSOR_DOWN":
      if (state.inputMode === "multi") {
        return withInput(state, moveDown(state.input));
      }
      return null;
    case "CURSOR_HOME":
      return withInput(state, moveToLineStart(state.input));
    case "CURSOR_END":
      return withInput(state, moveToLineEnd(state.input));
    case "INSERT_NEWLINE":
      if (state.inputMode === "multi") {
        return withInput(state, insertNewline(state.input));
      }
      return null;
    default:
      return null;
  }
};

const createTodoTransition = (state: AppState, action: Action): AppState => {
  const inputResult = handleInputAction(state, action);
  if (inputResult) return inputResult;

  switch (action.type) {
    case "BACK":
      return {
        ...state,
        view: "main_menu",
        inputBuffer: "",
        input: clearBuffer(),
        menuIndex: 0,
      };
    case "SUBMIT": {
      if (!state.inputBuffer.trim()) return state;
      const now = Date.now();
      const newTodo: Todo = {
        id: generateId(),
        title: state.inputBuffer.trim(),
        items: [],
        createdAt: now,
        updatedAt: now,
      };
      return {
        ...state,
        view: "view_todo",
        todos: [...state.todos, newTodo],
        selectedTodoId: newTodo.id,
        inputBuffer: "",
        input: clearBuffer(),
        menuIndex: 0,
      };
    }
    default:
      return state;
  }
};

const loadTodoTransition = (state: AppState, action: Action): AppState => {
  const todoCount = state.todos.length;

  switch (action.type) {
    case "BACK":
      return { ...state, view: "main_menu", menuIndex: 0 };
    case "NAVIGATE_DOWN":
      return {
        ...state,
        menuIndex: todoCount > 0 ? (state.menuIndex + 1) % todoCount : 0,
      };
    case "NAVIGATE_UP":
      return {
        ...state,
        menuIndex:
          todoCount > 0 ? (state.menuIndex - 1 + todoCount) % todoCount : 0,
      };
    case "SELECT": {
      const todo = state.todos[state.menuIndex];
      if (!todo) return state;
      return {
        ...state,
        view: "view_todo",
        selectedTodoId: todo.id,
        menuIndex: 0,
      };
    }
    case "ENTER_SEARCH":
      return {
        ...state,
        view: "search_todo",
        menuIndex: 0,
        searchQuery: "",
        inputBuffer: "",
        input: clearBuffer(),
      };
    case "TOGGLE_DATE_FILTER":
      return {
        ...state,
        dateFilter: nextDateFilter(state.dateFilter),
        menuIndex: 0,
      };
    default:
      return state;
  }
};

const searchTodoTransition = (state: AppState, action: Action): AppState => {
  switch (action.type) {
    case "BACK":
      return {
        ...state,
        view: "load_todo",
        menuIndex: 0,
        searchQuery: "",
        inputBuffer: "",
        input: clearBuffer(),
      };
    case "INPUT_CHAR": {
      const newInput = insertChar(state.input, action.char);
      const newQuery = getBufferText(newInput);
      return {
        ...state,
        input: newInput,
        inputBuffer: newQuery,
        searchQuery: newQuery,
        menuIndex: 0,
      };
    }
    case "INPUT_BACKSPACE": {
      const newInput = deleteBackward(state.input);
      const newQuery = getBufferText(newInput);
      return {
        ...state,
        input: newInput,
        inputBuffer: newQuery,
        searchQuery: newQuery,
        menuIndex: 0,
      };
    }
    case "DELETE_FORWARD": {
      const newInput = deleteForward(state.input);
      const newQuery = getBufferText(newInput);
      return {
        ...state,
        input: newInput,
        inputBuffer: newQuery,
        searchQuery: newQuery,
        menuIndex: 0,
      };
    }
    case "CURSOR_LEFT":
      return withInput(state, moveLeft(state.input));
    case "CURSOR_RIGHT":
      return withInput(state, moveRight(state.input));
    case "CURSOR_HOME":
      return withInput(state, moveToLineStart(state.input));
    case "CURSOR_END":
      return withInput(state, moveToLineEnd(state.input));
    case "NAVIGATE_DOWN":
    case "NAVIGATE_UP": {
      const { filteredTodos } = getFilteredTodos(state);
      const count = filteredTodos.length;
      if (count === 0) return state;
      const delta = action.type === "NAVIGATE_DOWN" ? 1 : -1;
      return {
        ...state,
        menuIndex: (state.menuIndex + delta + count) % count,
      };
    }
    case "SELECT": {
      const { filteredTodos } = getFilteredTodos(state);
      const entry = filteredTodos[state.menuIndex];
      if (!entry) return state;
      return {
        ...state,
        view: "view_todo",
        selectedTodoId: entry.id,
        menuIndex: 0,
        searchQuery: "",
        inputBuffer: "",
        input: clearBuffer(),
      };
    }
    case "TOGGLE_DATE_FILTER":
      return {
        ...state,
        dateFilter: nextDateFilter(state.dateFilter),
        menuIndex: 0,
      };
    default:
      return state;
  }
};

const getFilteredTodos = (
  state: AppState,
): { filteredTodos: { id: string; title: string }[] } => {
  const entries = state.todos.map((t) => ({
    id: t.id,
    title: t.title,
    createdAt: t.createdAt,
    updatedAt: t.updatedAt,
    itemCount: t.items.length,
    doneCount: t.items.filter((i) => i.done).length,
  }));
  const dateFiltered = filterByDate(entries, state.dateFilter);
  return { filteredTodos: filterTodos(dateFiltered, state.searchQuery) };
};

export { getFilteredTodos };

const viewTodoTransition = (state: AppState, action: Action): AppState => {
  const todo = state.todos.find((t) => t.id === state.selectedTodoId);
  if (!todo) return { ...state, view: "main_menu", menuIndex: 0 };

  const inputResult = handleInputAction(state, action);
  if (inputResult) return inputResult;

  switch (action.type) {
    case "BACK":
      return {
        ...state,
        view: "main_menu",
        selectedTodoId: null,
        menuIndex: 0,
        inputBuffer: "",
        input: clearBuffer(),
      };
    case "NAVIGATE_DOWN":
      return {
        ...state,
        menuIndex: (state.menuIndex + 1) % Math.max(1, todo.items.length),
      };
    case "NAVIGATE_UP":
      return {
        ...state,
        menuIndex:
          (state.menuIndex - 1 + Math.max(1, todo.items.length)) %
          Math.max(1, todo.items.length),
      };
    case "SELECT": {
      const item = todo.items[state.menuIndex];
      if (!item) return state;
      const updatedItems = todo.items.map((it, i) =>
        i === state.menuIndex ? { ...it, done: !it.done } : it,
      );
      const updatedTodo = {
        ...todo,
        items: updatedItems,
        updatedAt: Date.now(),
      };
      return {
        ...state,
        todos: state.todos.map((t) => (t.id === todo.id ? updatedTodo : t)),
      };
    }
    case "SUBMIT": {
      if (!state.inputBuffer.trim()) return state;
      const newItem: TodoItem = { text: state.inputBuffer.trim(), done: false };
      const updatedTodo = {
        ...todo,
        items: [...todo.items, newItem],
        updatedAt: Date.now(),
      };
      return {
        ...state,
        todos: state.todos.map((t) => (t.id === todo.id ? updatedTodo : t)),
        inputBuffer: "",
        input: clearBuffer(),
        menuIndex: updatedTodo.items.length - 1,
      };
    }
    case "MOVE_ITEM_UP": {
      if (state.menuIndex <= 0 || todo.items.length < 2) return state;
      const idx = state.menuIndex;
      const items = todo.items.map((item, i) =>
        i === idx - 1
          ? todo.items[idx]!
          : i === idx
            ? todo.items[idx - 1]!
            : item,
      );
      const updatedTodo = { ...todo, items, updatedAt: Date.now() };
      return {
        ...state,
        todos: state.todos.map((t) => (t.id === todo.id ? updatedTodo : t)),
        menuIndex: idx - 1,
      };
    }
    case "MOVE_ITEM_DOWN": {
      if (state.menuIndex >= todo.items.length - 1 || todo.items.length < 2)
        return state;
      const idx = state.menuIndex;
      const items = todo.items.map((item, i) =>
        i === idx
          ? todo.items[idx + 1]!
          : i === idx + 1
            ? todo.items[idx]!
            : item,
      );
      const updatedTodo = { ...todo, items, updatedAt: Date.now() };
      return {
        ...state,
        todos: state.todos.map((t) => (t.id === todo.id ? updatedTodo : t)),
        menuIndex: idx + 1,
      };
    }
    default:
      return state;
  }
};

export const transition = (state: AppState, action: Action): AppState => {
  if (action.type === "QUIT") {
    return { ...state, view: "quit" };
  }

  if (action.type === "LOAD_TODOS") {
    return { ...state, todos: action.todos };
  }

  if (action.type === "RESIZE") {
    return {
      ...state,
      terminalSize: { rows: action.rows, cols: action.cols },
    };
  }

  switch (state.view) {
    case "main_menu":
      return mainMenuTransition(state, action);
    case "create_todo":
      return createTodoTransition(state, action);
    case "load_todo":
      return loadTodoTransition(state, action);
    case "search_todo":
      return searchTodoTransition(state, action);
    case "view_todo":
      return viewTodoTransition(state, action);
    default:
      return state;
  }
};
