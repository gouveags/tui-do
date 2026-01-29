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
import {
  type AppState,
  createInputBuffer,
  type InputBuffer,
  type Todo,
  type TodoItem,
} from "./types.ts";

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
  | { type: "TOGGLE_DATE_FILTER" }
  | { type: "TOGGLE_SELECTION_MODE" }
  | { type: "TOGGLE_SELECTION" }
  | { type: "REQUEST_DELETE" }
  | { type: "REQUEST_RENAME" }
  | { type: "CONFIRM_MODAL" }
  | { type: "CANCEL_MODAL" };

const MAIN_MENU_OPTIONS = 3;

const generateId = (): string =>
  Date.now().toString(36) + Math.random().toString(36).slice(2, 7);

const syncInputBuffer = (state: AppState): AppState => ({
  ...state,
  inputBuffer: getBufferText(state.input),
});

const syncModalInputBuffer = (state: AppState): AppState => ({
  ...state,
  modalInputBuffer: getBufferText(state.modalInput),
});

const withInput = (state: AppState, input: InputBuffer): AppState =>
  syncInputBuffer({ ...state, input });

const withModalInput = (state: AppState, modalInput: InputBuffer): AppState =>
  syncModalInputBuffer({ ...state, modalInput });

const getTodoEntries = (state: AppState) =>
  state.todos.map((t) => ({
    id: t.id,
    title: t.title,
    createdAt: t.createdAt,
    updatedAt: t.updatedAt,
    itemCount: t.items.length,
    doneCount: t.items.filter((i) => i.done).length,
  }));

const getVisibleTodos = (
  state: AppState,
  query: string = state.searchQuery,
) => {
  const dateFiltered = filterByDate(getTodoEntries(state), state.dateFilter);
  return filterTodos(dateFiltered, query);
};

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

const handleModalInputAction = (
  state: AppState,
  action: Action,
): AppState | null => {
  switch (action.type) {
    case "INPUT_CHAR":
      return withModalInput(state, insertChar(state.modalInput, action.char));
    case "INPUT_BACKSPACE":
      return withModalInput(state, deleteBackward(state.modalInput));
    case "DELETE_FORWARD":
      return withModalInput(state, deleteForward(state.modalInput));
    case "CURSOR_LEFT":
      return withModalInput(state, moveLeft(state.modalInput));
    case "CURSOR_RIGHT":
      return withModalInput(state, moveRight(state.modalInput));
    case "CURSOR_HOME":
      return withModalInput(state, moveToLineStart(state.modalInput));
    case "CURSOR_END":
      return withModalInput(state, moveToLineEnd(state.modalInput));
    default:
      return null;
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

const closeModal = (state: AppState): AppState => ({
  ...state,
  modal: null,
  modalInput: clearBuffer(),
  modalInputBuffer: "",
});

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
  const entries = getVisibleTodos(state, "");
  const todoCount = entries.length;

  switch (action.type) {
    case "BACK":
      return {
        ...state,
        view: "main_menu",
        menuIndex: 0,
        selectionMode: false,
        selectedTodoIds: [],
      };
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
      if (state.selectionMode) {
        const entry = entries[state.menuIndex];
        if (!entry) return state;
        const isSelected = state.selectedTodoIds.includes(entry.id);
        return {
          ...state,
          selectedTodoIds: isSelected
            ? state.selectedTodoIds.filter((id) => id !== entry.id)
            : [...state.selectedTodoIds, entry.id],
        };
      }
      const entry = entries[state.menuIndex];
      if (!entry) return state;
      return {
        ...state,
        view: "view_todo",
        selectedTodoId: entry.id,
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
        selectionMode: false,
        selectedTodoIds: [],
      };
    case "TOGGLE_DATE_FILTER":
      return {
        ...state,
        dateFilter: nextDateFilter(state.dateFilter),
        menuIndex: 0,
      };
    case "TOGGLE_SELECTION_MODE":
      return {
        ...state,
        selectionMode: !state.selectionMode,
        selectedTodoIds: state.selectionMode ? [] : state.selectedTodoIds,
      };
    case "TOGGLE_SELECTION": {
      const entry = entries[state.menuIndex];
      if (!entry) return state;
      const isSelected = state.selectedTodoIds.includes(entry.id);
      return {
        ...state,
        selectedTodoIds: isSelected
          ? state.selectedTodoIds.filter((id) => id !== entry.id)
          : [...state.selectedTodoIds, entry.id],
      };
    }
    case "REQUEST_DELETE": {
      if (state.selectionMode && state.selectedTodoIds.length > 0) {
        return {
          ...state,
          modal: { type: "confirm_delete", todoIds: state.selectedTodoIds },
        };
      }
      const entry = entries[state.menuIndex];
      if (!entry) return state;
      return {
        ...state,
        modal: { type: "confirm_delete", todoIds: [entry.id] },
      };
    }
    case "REQUEST_RENAME": {
      const entry = entries[state.menuIndex];
      if (!entry) return state;
      return {
        ...state,
        modal: { type: "rename", todoId: entry.id },
        modalInput: createInputBuffer(entry.title),
        modalInputBuffer: entry.title,
      };
    }
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
        selectionMode: false,
        selectedTodoIds: [],
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
      const entries = getVisibleTodos(state);
      const count = entries.length;
      if (count === 0) return state;
      const delta = action.type === "NAVIGATE_DOWN" ? 1 : -1;
      return {
        ...state,
        menuIndex: (state.menuIndex + delta + count) % count,
      };
    }
    case "SELECT": {
      if (state.selectionMode) {
        const entry = getVisibleTodos(state)[state.menuIndex];
        if (!entry) return state;
        const isSelected = state.selectedTodoIds.includes(entry.id);
        return {
          ...state,
          selectedTodoIds: isSelected
            ? state.selectedTodoIds.filter((id) => id !== entry.id)
            : [...state.selectedTodoIds, entry.id],
        };
      }
      const entry = getVisibleTodos(state)[state.menuIndex];
      if (!entry) return state;
      return {
        ...state,
        view: "view_todo",
        selectedTodoId: entry.id,
        menuIndex: 0,
        searchQuery: "",
        inputBuffer: "",
        input: clearBuffer(),
        selectionMode: false,
        selectedTodoIds: [],
      };
    }
    case "TOGGLE_DATE_FILTER":
      return {
        ...state,
        dateFilter: nextDateFilter(state.dateFilter),
        menuIndex: 0,
      };
    case "TOGGLE_SELECTION_MODE":
      return {
        ...state,
        selectionMode: !state.selectionMode,
        selectedTodoIds: state.selectionMode ? [] : state.selectedTodoIds,
      };
    case "TOGGLE_SELECTION": {
      const entry = getVisibleTodos(state)[state.menuIndex];
      if (!entry) return state;
      const isSelected = state.selectedTodoIds.includes(entry.id);
      return {
        ...state,
        selectedTodoIds: isSelected
          ? state.selectedTodoIds.filter((id) => id !== entry.id)
          : [...state.selectedTodoIds, entry.id],
      };
    }
    case "REQUEST_DELETE": {
      if (state.selectionMode && state.selectedTodoIds.length > 0) {
        return {
          ...state,
          modal: { type: "confirm_delete", todoIds: state.selectedTodoIds },
        };
      }
      const entry = getVisibleTodos(state)[state.menuIndex];
      if (!entry) return state;
      return {
        ...state,
        modal: { type: "confirm_delete", todoIds: [entry.id] },
      };
    }
    case "REQUEST_RENAME": {
      const entry = getVisibleTodos(state)[state.menuIndex];
      if (!entry) return state;
      return {
        ...state,
        modal: { type: "rename", todoId: entry.id },
        modalInput: createInputBuffer(entry.title),
        modalInputBuffer: entry.title,
      };
    }
    default:
      return state;
  }
};

export { getVisibleTodos };

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

  if (state.modal) {
    if (action.type === "CANCEL_MODAL") {
      return closeModal(state);
    }
    if (action.type === "CONFIRM_MODAL") {
      if (state.modal.type === "confirm_delete") {
        const remainingTodos = state.todos.filter(
          (todo) => !state.modal?.todoIds.includes(todo.id),
        );
        const menuIndex = Math.min(
          state.menuIndex,
          Math.max(0, remainingTodos.length - 1),
        );
        return closeModal({
          ...state,
          todos: remainingTodos,
          selectedTodoId: state.modal.todoIds.includes(
            state.selectedTodoId ?? "",
          )
            ? null
            : state.selectedTodoId,
          menuIndex,
          selectionMode: false,
          selectedTodoIds: [],
        });
      }
      if (state.modal.type === "rename") {
        const newTitle = state.modalInputBuffer.trim();
        if (!newTitle) return closeModal(state);
        const now = Date.now();
        const updatedTodos = state.todos.map((todo) =>
          todo.id === state.modal?.todoId
            ? { ...todo, title: newTitle, updatedAt: now }
            : todo,
        );
        return closeModal({ ...state, todos: updatedTodos });
      }
    }
    if (state.modal.type === "rename") {
      const inputResult = handleModalInputAction(state, action);
      if (inputResult) return inputResult;
    }
    return state;
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
