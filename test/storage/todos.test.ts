import assert from "node:assert";
import fs from "node:fs";
import path from "node:path";
import { after, before, beforeEach, describe, it } from "node:test";
import type { Todo } from "../../src/state/types.ts";
import {
  deleteTodo,
  ensureStorageDir,
  listTodos,
  loadTodo,
  saveTodo,
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
  });
});
