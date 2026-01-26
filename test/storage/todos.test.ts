import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import { after, before, beforeEach, describe, it } from "node:test";
import type { Todo } from "../../src/state/types.ts";
import {
  deleteTodo,
  ensureStorageDir,
  INDEX_FILE,
  listTodos,
  listTodosFromIndex,
  loadTodo,
  readIndex,
  rebuildIndex,
  saveTodo,
  updateIndex,
  writeIndex,
} from "../../src/storage/todos.ts";

const TEST_DIR = "./test-todos";

describe("storage", () => {
  before(() => {
    if (fs.existsSync(TEST_DIR)) {
      fs.rmSync(TEST_DIR, { recursive: true });
    }
  });

  after(() => {
    if (fs.existsSync(TEST_DIR)) {
      fs.rmSync(TEST_DIR, { recursive: true });
    }
  });

  beforeEach(() => {
    if (fs.existsSync(TEST_DIR)) {
      fs.rmSync(TEST_DIR, { recursive: true });
    }
  });

  describe("ensureStorageDir", () => {
    it("creates directory if not exists", () => {
      ensureStorageDir(TEST_DIR);
      assert.ok(fs.existsSync(TEST_DIR));
    });

    it("does not throw if directory exists", () => {
      fs.mkdirSync(TEST_DIR, { recursive: true });
      assert.doesNotThrow(() => ensureStorageDir(TEST_DIR));
    });
  });

  describe("saveTodo", () => {
    it("saves todo to file", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "test-1",
        title: "Test Todo",
        items: [{ text: "Item 1", done: false }],
        createdAt: 1000,
        updatedAt: 2000,
      };
      saveTodo(todo, TEST_DIR);
      const filePath = path.join(TEST_DIR, "test-1.json");
      assert.ok(fs.existsSync(filePath));
    });

    it("saves todo with correct content", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "test-2",
        title: "Another Todo",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      saveTodo(todo, TEST_DIR);
      const content = fs.readFileSync(
        path.join(TEST_DIR, "test-2.json"),
        "utf8",
      );
      const parsed = JSON.parse(content);
      assert.strictEqual(parsed.title, "Another Todo");
    });
  });

  describe("loadTodo", () => {
    it("loads todo from file", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "test-3",
        title: "Load Test",
        items: [{ text: "Test item", done: true }],
        createdAt: 1000,
        updatedAt: 2000,
      };
      saveTodo(todo, TEST_DIR);
      const loaded = loadTodo("test-3", TEST_DIR);
      assert.deepStrictEqual(loaded, todo);
    });

    it("returns null for non-existent todo", () => {
      ensureStorageDir(TEST_DIR);
      const loaded = loadTodo("non-existent", TEST_DIR);
      assert.strictEqual(loaded, null);
    });
  });

  describe("listTodos", () => {
    it("returns empty array when no todos", () => {
      ensureStorageDir(TEST_DIR);
      const todos = listTodos(TEST_DIR);
      assert.deepStrictEqual(todos, []);
    });

    it("returns all saved todos", () => {
      ensureStorageDir(TEST_DIR);
      const todo1: Todo = {
        id: "t1",
        title: "First",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      const todo2: Todo = {
        id: "t2",
        title: "Second",
        items: [],
        createdAt: 2000,
        updatedAt: 3000,
      };
      saveTodo(todo1, TEST_DIR);
      saveTodo(todo2, TEST_DIR);
      const todos = listTodos(TEST_DIR);
      assert.strictEqual(todos.length, 2);
    });

    it("returns todos sorted by updatedAt descending", () => {
      ensureStorageDir(TEST_DIR);
      const todo1: Todo = {
        id: "t1",
        title: "Older",
        items: [],
        createdAt: 1000,
        updatedAt: 1000,
      };
      const todo2: Todo = {
        id: "t2",
        title: "Newer",
        items: [],
        createdAt: 2000,
        updatedAt: 3000,
      };
      saveTodo(todo1, TEST_DIR);
      saveTodo(todo2, TEST_DIR);
      const todos = listTodos(TEST_DIR);
      assert.strictEqual(todos[0]?.title, "Newer");
    });
  });

  describe("deleteTodo", () => {
    it("deletes todo file", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "del-1",
        title: "To Delete",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      saveTodo(todo, TEST_DIR);
      deleteTodo("del-1", TEST_DIR);
      assert.ok(!fs.existsSync(path.join(TEST_DIR, "del-1.json")));
    });

    it("does not throw for non-existent todo", () => {
      ensureStorageDir(TEST_DIR);
      assert.doesNotThrow(() => deleteTodo("non-existent", TEST_DIR));
    });

    it("removes entry from index when deleting", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "del-idx",
        title: "To Delete",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      saveTodo(todo, TEST_DIR);
      deleteTodo("del-idx", TEST_DIR);
      const index = readIndex(TEST_DIR);
      assert.strictEqual(
        index?.entries.find((e) => e.id === "del-idx"),
        undefined,
      );
    });
  });

  describe("index", () => {
    it("readIndex returns null when no index exists", () => {
      ensureStorageDir(TEST_DIR);
      const index = readIndex(TEST_DIR);
      assert.strictEqual(index, null);
    });

    it("writeIndex creates index file", () => {
      ensureStorageDir(TEST_DIR);
      writeIndex({ version: 1, entries: [], lastUpdated: 1000 }, TEST_DIR);
      assert.ok(fs.existsSync(path.join(TEST_DIR, INDEX_FILE)));
    });

    it("saveTodo updates index", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "idx-1",
        title: "Index Test",
        items: [
          { text: "Item 1", done: true },
          { text: "Item 2", done: false },
        ],
        createdAt: 1000,
        updatedAt: 2000,
      };
      saveTodo(todo, TEST_DIR);
      const index = readIndex(TEST_DIR);
      assert.ok(index);
      const entry = index.entries.find((e) => e.id === "idx-1");
      assert.ok(entry);
      assert.strictEqual(entry.title, "Index Test");
      assert.strictEqual(entry.itemCount, 2);
      assert.strictEqual(entry.doneCount, 1);
    });

    it("updateIndex adds new entry", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "upd-1",
        title: "Update Test",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      updateIndex(todo, TEST_DIR);
      const index = readIndex(TEST_DIR);
      assert.ok(index?.entries.find((e) => e.id === "upd-1"));
    });

    it("updateIndex updates existing entry", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "upd-2",
        title: "Original",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      updateIndex(todo, TEST_DIR);
      todo.title = "Updated";
      updateIndex(todo, TEST_DIR);
      const index = readIndex(TEST_DIR);
      const entries = index?.entries.filter((e) => e.id === "upd-2");
      assert.strictEqual(entries?.length, 1);
      assert.strictEqual(entries?.[0]?.title, "Updated");
    });

    it("rebuildIndex creates index from existing files", () => {
      ensureStorageDir(TEST_DIR);
      const todo1: Todo = {
        id: "rb-1",
        title: "Rebuild 1",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      const todo2: Todo = {
        id: "rb-2",
        title: "Rebuild 2",
        items: [{ text: "Item", done: true }],
        createdAt: 2000,
        updatedAt: 3000,
      };
      fs.writeFileSync(path.join(TEST_DIR, "rb-1.json"), JSON.stringify(todo1));
      fs.writeFileSync(path.join(TEST_DIR, "rb-2.json"), JSON.stringify(todo2));
      const index = rebuildIndex(TEST_DIR);
      assert.strictEqual(index.entries.length, 2);
    });

    it("listTodosFromIndex returns sorted entries", () => {
      ensureStorageDir(TEST_DIR);
      const todo1: Todo = {
        id: "list-1",
        title: "Older",
        items: [],
        createdAt: 1000,
        updatedAt: 1000,
      };
      const todo2: Todo = {
        id: "list-2",
        title: "Newer",
        items: [],
        createdAt: 2000,
        updatedAt: 3000,
      };
      saveTodo(todo1, TEST_DIR);
      saveTodo(todo2, TEST_DIR);
      const entries = listTodosFromIndex(TEST_DIR);
      assert.strictEqual(entries[0]?.title, "Newer");
    });

    it("listTodosFromIndex rebuilds missing index", () => {
      ensureStorageDir(TEST_DIR);
      const todo: Todo = {
        id: "auto-rb",
        title: "Auto Rebuild",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      fs.writeFileSync(
        path.join(TEST_DIR, "auto-rb.json"),
        JSON.stringify(todo),
      );
      const entries = listTodosFromIndex(TEST_DIR);
      assert.ok(entries.find((e) => e.id === "auto-rb"));
      assert.ok(fs.existsSync(path.join(TEST_DIR, INDEX_FILE)));
    });

    it("listTodos excludes index file", () => {
      ensureStorageDir(TEST_DIR);
      writeIndex({ version: 1, entries: [], lastUpdated: 1000 }, TEST_DIR);
      const todo: Todo = {
        id: "excl-1",
        title: "Exclude Test",
        items: [],
        createdAt: 1000,
        updatedAt: 2000,
      };
      saveTodo(todo, TEST_DIR);
      const todos = listTodos(TEST_DIR);
      assert.strictEqual(todos.length, 1);
      assert.strictEqual(todos[0]?.id, "excl-1");
    });
  });
});
