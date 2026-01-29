export type Todo = {
  id: string;
  title: string;
  items: TodoItem[];
  createdAt: number;
  updatedAt: number;
};

export type TodoItem = {
  text: string;
  done: boolean;
};

export type TodoIndexEntry = {
  id: string;
  title: string;
  createdAt: number;
  updatedAt: number;
  itemCount: number;
  doneCount: number;
};

export type TodoIndex = {
  version: 1;
  entries: TodoIndexEntry[];
  lastUpdated: number;
};

export type DateFilter = "all" | "today" | "week" | "month";

export type CursorPosition = {
  line: number;
  col: number;
};

export type InputBuffer = {
  lines: string[];
  cursor: CursorPosition;
};

export type InputMode = "single" | "multi";

export type TerminalSize = {
  rows: number;
  cols: number;
};

export type AppView =
  | "main_menu"
  | "create_todo"
  | "load_todo"
  | "search_todo"
  | "view_todo"
  | "quit";

export type ModalState =
  | { type: "confirm_delete"; todoIds: string[] }
  | { type: "rename"; todoId: string };

export type AppState = {
  view: AppView;
  menuIndex: number;
  todos: Todo[];
  selectedTodoId: string | null;
  inputBuffer: string;
  input: InputBuffer;
  inputMode: InputMode;
  terminalSize: TerminalSize;
  message: string | null;
  searchQuery: string;
  dateFilter: DateFilter;
  selectionMode: boolean;
  selectedTodoIds: string[];
  modal: ModalState | null;
  modalInput: InputBuffer;
  modalInputBuffer: string;
};

export const createInputBuffer = (text = ""): InputBuffer => ({
  lines: [text],
  cursor: { line: 0, col: text.length },
});

export const createInitialState = (): AppState => ({
  view: "main_menu",
  menuIndex: 0,
  todos: [],
  selectedTodoId: null,
  inputBuffer: "",
  input: createInputBuffer(),
  inputMode: "single",
  terminalSize: { rows: 24, cols: 80 },
  message: null,
  searchQuery: "",
  dateFilter: "all",
  selectionMode: false,
  selectedTodoIds: [],
  modal: null,
  modalInput: createInputBuffer(),
  modalInputBuffer: "",
});
