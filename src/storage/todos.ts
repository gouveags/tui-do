import fs from "node:fs";
import path from "node:path";
import type { Todo } from "../state/types.ts";

export const STORAGE_DIR = "./to-dos";

export const ensureStorageDir = (dir: string = STORAGE_DIR): void => {
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
  }
};

export const saveTodo = (todo: Todo, dir: string = STORAGE_DIR): void => {
  const filePath = path.join(dir, `${todo.id}.json`);
  fs.writeFileSync(filePath, JSON.stringify(todo, null, 2));
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
  const files = fs.readdirSync(dir).filter((f) => f.endsWith(".json"));
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
};
