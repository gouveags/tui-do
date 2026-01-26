import fs from "node:fs";
import path from "node:path";
import type { Todo, TodoIndex, TodoIndexEntry } from "../state/types.ts";

export const STORAGE_DIR = "./to-dos";
export const INDEX_FILE = ".index.json";

export const ensureStorageDir = (dir: string = STORAGE_DIR): void => {
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
  }
};

export const saveTodo = (todo: Todo, dir: string = STORAGE_DIR): void => {
  const filePath = path.join(dir, `${todo.id}.json`);
  fs.writeFileSync(filePath, JSON.stringify(todo, null, 2));
  updateIndex(todo, dir);
};

export const loadTodo = (
  id: string,
  dir: string = STORAGE_DIR,
): Todo | null => {
  const filePath = path.join(dir, `${id}.json`);
  if (!fs.existsSync(filePath)) return null;
  const content = fs.readFileSync(filePath, "utf8");
  return JSON.parse(content) as Todo;
};

export const listTodos = (dir: string = STORAGE_DIR): Todo[] => {
  if (!fs.existsSync(dir)) return [];
  const files = fs
    .readdirSync(dir)
    .filter((f) => f.endsWith(".json") && f !== INDEX_FILE);
  const todos: Todo[] = [];

  for (const file of files) {
    const content = fs.readFileSync(path.join(dir, file), "utf8");
    todos.push(JSON.parse(content) as Todo);
  }

  return todos.sort((a, b) => b.updatedAt - a.updatedAt);
};

export const deleteTodo = (id: string, dir: string = STORAGE_DIR): void => {
  const filePath = path.join(dir, `${id}.json`);
  if (fs.existsSync(filePath)) {
    fs.unlinkSync(filePath);
  }
  removeFromIndex(id, dir);
};

const todoToIndexEntry = (todo: Todo): TodoIndexEntry => ({
  id: todo.id,
  title: todo.title,
  createdAt: todo.createdAt,
  updatedAt: todo.updatedAt,
  itemCount: todo.items.length,
  doneCount: todo.items.filter((i) => i.done).length,
});

const createEmptyIndex = (): TodoIndex => ({
  version: 1,
  entries: [],
  lastUpdated: Date.now(),
});

export const readIndex = (dir: string = STORAGE_DIR): TodoIndex | null => {
  const indexPath = path.join(dir, INDEX_FILE);
  if (!fs.existsSync(indexPath)) return null;
  const content = fs.readFileSync(indexPath, "utf8");
  return JSON.parse(content) as TodoIndex;
};

export const writeIndex = (
  index: TodoIndex,
  dir: string = STORAGE_DIR,
): void => {
  ensureStorageDir(dir);
  const indexPath = path.join(dir, INDEX_FILE);
  fs.writeFileSync(indexPath, JSON.stringify(index, null, 2));
};

export const updateIndex = (todo: Todo, dir: string = STORAGE_DIR): void => {
  const index = readIndex(dir) ?? createEmptyIndex();
  const entry = todoToIndexEntry(todo);
  const existingIdx = index.entries.findIndex((e) => e.id === todo.id);
  if (existingIdx >= 0) {
    index.entries[existingIdx] = entry;
  } else {
    index.entries.push(entry);
  }
  index.lastUpdated = Date.now();
  writeIndex(index, dir);
};

const removeFromIndex = (id: string, dir: string = STORAGE_DIR): void => {
  const index = readIndex(dir);
  if (!index) return;
  index.entries = index.entries.filter((e) => e.id !== id);
  index.lastUpdated = Date.now();
  writeIndex(index, dir);
};

export const rebuildIndex = (dir: string = STORAGE_DIR): TodoIndex => {
  const todos = listTodosRaw(dir);
  const index: TodoIndex = {
    version: 1,
    entries: todos.map(todoToIndexEntry),
    lastUpdated: Date.now(),
  };
  writeIndex(index, dir);
  return index;
};

const listTodosRaw = (dir: string): Todo[] => {
  if (!fs.existsSync(dir)) return [];
  const files = fs
    .readdirSync(dir)
    .filter((f) => f.endsWith(".json") && f !== INDEX_FILE);
  const todos: Todo[] = [];
  for (const file of files) {
    const content = fs.readFileSync(path.join(dir, file), "utf8");
    todos.push(JSON.parse(content) as Todo);
  }
  return todos;
};

export const listTodosFromIndex = (
  dir: string = STORAGE_DIR,
): TodoIndexEntry[] => {
  let index = readIndex(dir);
  if (!index) {
    index = rebuildIndex(dir);
  }
  return index.entries.sort((a, b) => b.updatedAt - a.updatedAt);
};
