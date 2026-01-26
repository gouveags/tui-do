import assert from "node:assert";
import { describe, it } from "node:test";
import { transition } from "../../src/state/app.ts";
import { createInitialState } from "../../src/state/types.ts";

describe("state transitions", () => {
  describe("main_menu", () => {
    it("navigates down", () => {
      const state = createInitialState();
      const next = transition(state, { type: "NAVIGATE_DOWN" });
      assert.strictEqual(next.menuIndex, 1);
    });

    it("navigates up", () => {
      const state = { ...createInitialState(), menuIndex: 1 };
      const next = transition(state, { type: "NAVIGATE_UP" });
      assert.strictEqual(next.menuIndex, 0);
    });

    it("wraps navigation at boundaries", () => {
      const state = { ...createInitialState(), menuIndex: 2 };
      const next = transition(state, { type: "NAVIGATE_DOWN" });
      assert.strictEqual(next.menuIndex, 0);

      const state2 = createInitialState();
      const next2 = transition(state2, { type: "NAVIGATE_UP" });
      assert.strictEqual(next2.menuIndex, 2);
    });

    it("selects create todo with Enter", () => {
      const state = createInitialState();
      const next = transition(state, { type: "SELECT" });
      assert.strictEqual(next.view, "create_todo");
    });

    it("selects load todo with Enter on option 2", () => {
      const state = { ...createInitialState(), menuIndex: 1 };
      const next = transition(state, { type: "SELECT" });
      assert.strictEqual(next.view, "load_todo");
    });

    it("selects quit with Enter on option 3", () => {
      const state = { ...createInitialState(), menuIndex: 2 };
      const next = transition(state, { type: "SELECT" });
      assert.strictEqual(next.view, "quit");
    });

    it("quick selects with number keys", () => {
      const state = createInitialState();

      const next1 = transition(state, { type: "QUICK_SELECT", option: 1 });
      assert.strictEqual(next1.view, "create_todo");

      const next2 = transition(state, { type: "QUICK_SELECT", option: 2 });
      assert.strictEqual(next2.view, "load_todo");

      const next3 = transition(state, { type: "QUICK_SELECT", option: 3 });
      assert.strictEqual(next3.view, "quit");
    });

    it("transitions to quit on QUIT action", () => {
      const state = createInitialState();
      const next = transition(state, { type: "QUIT" });
      assert.strictEqual(next.view, "quit");
    });
  });

  describe("create_todo", () => {
    it("returns to main_menu on BACK", () => {
      const state = { ...createInitialState(), view: "create_todo" as const };
      const next = transition(state, { type: "BACK" });
      assert.strictEqual(next.view, "main_menu");
    });

    it("adds character to input buffer", () => {
      const state = { ...createInitialState(), view: "create_todo" as const };
      const next = transition(state, { type: "INPUT_CHAR", char: "a" });
      assert.strictEqual(next.inputBuffer, "a");
      assert.strictEqual(next.input.lines[0], "a");
    });

    it("removes character on backspace", () => {
      const state = {
        ...createInitialState(),
        view: "create_todo" as const,
        inputBuffer: "hello",
        input: { lines: ["hello"], cursor: { line: 0, col: 5 } },
      };
      const next = transition(state, { type: "INPUT_BACKSPACE" });
      assert.strictEqual(next.inputBuffer, "hell");
    });

    it("moves cursor left", () => {
      const state = {
        ...createInitialState(),
        view: "create_todo" as const,
        input: { lines: ["hello"], cursor: { line: 0, col: 3 } },
      };
      const next = transition(state, { type: "CURSOR_LEFT" });
      assert.strictEqual(next.input.cursor.col, 2);
    });

    it("moves cursor right", () => {
      const state = {
        ...createInitialState(),
        view: "create_todo" as const,
        input: { lines: ["hello"], cursor: { line: 0, col: 2 } },
      };
      const next = transition(state, { type: "CURSOR_RIGHT" });
      assert.strictEqual(next.input.cursor.col, 3);
    });

    it("moves cursor to line start with HOME", () => {
      const state = {
        ...createInitialState(),
        view: "create_todo" as const,
        input: { lines: ["hello"], cursor: { line: 0, col: 3 } },
      };
      const next = transition(state, { type: "CURSOR_HOME" });
      assert.strictEqual(next.input.cursor.col, 0);
    });

    it("moves cursor to line end with END", () => {
      const state = {
        ...createInitialState(),
        view: "create_todo" as const,
        input: { lines: ["hello"], cursor: { line: 0, col: 2 } },
      };
      const next = transition(state, { type: "CURSOR_END" });
      assert.strictEqual(next.input.cursor.col, 5);
    });

    it("deletes forward with DELETE", () => {
      const state = {
        ...createInitialState(),
        view: "create_todo" as const,
        inputBuffer: "helllo",
        input: { lines: ["helllo"], cursor: { line: 0, col: 3 } },
      };
      const next = transition(state, { type: "DELETE_FORWARD" });
      assert.strictEqual(next.inputBuffer, "hello");
    });

    it("creates todo on SUBMIT with non-empty buffer", () => {
      const state = {
        ...createInitialState(),
        view: "create_todo" as const,
        inputBuffer: "My Todo",
      };
      const next = transition(state, { type: "SUBMIT" });
      assert.strictEqual(next.view, "view_todo");
      assert.strictEqual(next.todos.length, 1);
      assert.strictEqual(next.todos[0]?.title, "My Todo");
      assert.strictEqual(next.inputBuffer, "");
    });

    it("does not create todo on SUBMIT with empty buffer", () => {
      const state = {
        ...createInitialState(),
        view: "create_todo" as const,
        inputBuffer: "",
      };
      const next = transition(state, { type: "SUBMIT" });
      assert.strictEqual(next.view, "create_todo");
      assert.strictEqual(next.todos.length, 0);
    });
  });

  describe("load_todo", () => {
    it("returns to main_menu on BACK", () => {
      const state = { ...createInitialState(), view: "load_todo" as const };
      const next = transition(state, { type: "BACK" });
      assert.strictEqual(next.view, "main_menu");
    });

    it("navigates through todo list", () => {
      const todos = [
        { id: "1", title: "First", items: [], createdAt: 1, updatedAt: 1 },
        { id: "2", title: "Second", items: [], createdAt: 2, updatedAt: 2 },
      ];
      const state = {
        ...createInitialState(),
        view: "load_todo" as const,
        todos,
        menuIndex: 0,
      };

      const next = transition(state, { type: "NAVIGATE_DOWN" });
      assert.strictEqual(next.menuIndex, 1);
    });

    it("selects todo and transitions to view_todo", () => {
      const todos = [
        { id: "1", title: "First", items: [], createdAt: 1, updatedAt: 1 },
      ];
      const state = {
        ...createInitialState(),
        view: "load_todo" as const,
        todos,
        menuIndex: 0,
      };
      const next = transition(state, { type: "SELECT" });
      assert.strictEqual(next.view, "view_todo");
      assert.strictEqual(next.selectedTodoId, "1");
    });
  });

  describe("view_todo", () => {
    it("returns to main_menu on BACK", () => {
      const state = { ...createInitialState(), view: "view_todo" as const };
      const next = transition(state, { type: "BACK" });
      assert.strictEqual(next.view, "main_menu");
    });

    it("toggles todo item on SELECT", () => {
      const todos = [
        {
          id: "1",
          title: "Test",
          items: [{ text: "Item 1", done: false }],
          createdAt: 1,
          updatedAt: 1,
        },
      ];
      const state = {
        ...createInitialState(),
        view: "view_todo" as const,
        todos,
        selectedTodoId: "1",
        menuIndex: 0,
      };
      const next = transition(state, { type: "SELECT" });
      assert.strictEqual(next.todos[0]?.items[0]?.done, true);
    });

    it("adds new item on SUBMIT with input", () => {
      const todos = [
        { id: "1", title: "Test", items: [], createdAt: 1, updatedAt: 1 },
      ];
      const state = {
        ...createInitialState(),
        view: "view_todo" as const,
        todos,
        selectedTodoId: "1",
        inputBuffer: "New item",
        input: { lines: ["New item"], cursor: { line: 0, col: 8 } },
      };
      const next = transition(state, { type: "SUBMIT" });
      assert.strictEqual(next.todos[0]?.items.length, 1);
      assert.strictEqual(next.todos[0]?.items[0]?.text, "New item");
      assert.strictEqual(next.inputBuffer, "");
    });

    it("moves item up with MOVE_ITEM_UP", () => {
      const todos = [
        {
          id: "1",
          title: "Test",
          items: [
            { text: "First", done: false },
            { text: "Second", done: false },
            { text: "Third", done: false },
          ],
          createdAt: 1,
          updatedAt: 1,
        },
      ];
      const state = {
        ...createInitialState(),
        view: "view_todo" as const,
        todos,
        selectedTodoId: "1",
        menuIndex: 1,
      };
      const next = transition(state, { type: "MOVE_ITEM_UP" });
      assert.strictEqual(next.todos[0]?.items[0]?.text, "Second");
      assert.strictEqual(next.todos[0]?.items[1]?.text, "First");
      assert.strictEqual(next.menuIndex, 0);
    });

    it("does not move first item up", () => {
      const todos = [
        {
          id: "1",
          title: "Test",
          items: [
            { text: "First", done: false },
            { text: "Second", done: false },
          ],
          createdAt: 1,
          updatedAt: 1,
        },
      ];
      const state = {
        ...createInitialState(),
        view: "view_todo" as const,
        todos,
        selectedTodoId: "1",
        menuIndex: 0,
      };
      const next = transition(state, { type: "MOVE_ITEM_UP" });
      assert.strictEqual(next.todos[0]?.items[0]?.text, "First");
      assert.strictEqual(next.menuIndex, 0);
    });

    it("moves item down with MOVE_ITEM_DOWN", () => {
      const todos = [
        {
          id: "1",
          title: "Test",
          items: [
            { text: "First", done: false },
            { text: "Second", done: false },
            { text: "Third", done: false },
          ],
          createdAt: 1,
          updatedAt: 1,
        },
      ];
      const state = {
        ...createInitialState(),
        view: "view_todo" as const,
        todos,
        selectedTodoId: "1",
        menuIndex: 1,
      };
      const next = transition(state, { type: "MOVE_ITEM_DOWN" });
      assert.strictEqual(next.todos[0]?.items[1]?.text, "Third");
      assert.strictEqual(next.todos[0]?.items[2]?.text, "Second");
      assert.strictEqual(next.menuIndex, 2);
    });

    it("does not move last item down", () => {
      const todos = [
        {
          id: "1",
          title: "Test",
          items: [
            { text: "First", done: false },
            { text: "Second", done: false },
          ],
          createdAt: 1,
          updatedAt: 1,
        },
      ];
      const state = {
        ...createInitialState(),
        view: "view_todo" as const,
        todos,
        selectedTodoId: "1",
        menuIndex: 1,
      };
      const next = transition(state, { type: "MOVE_ITEM_DOWN" });
      assert.strictEqual(next.todos[0]?.items[1]?.text, "Second");
      assert.strictEqual(next.menuIndex, 1);
    });
  });

  describe("resize", () => {
    it("updates terminal size on RESIZE", () => {
      const state = createInitialState();
      const next = transition(state, { type: "RESIZE", rows: 40, cols: 120 });
      assert.strictEqual(next.terminalSize.rows, 40);
      assert.strictEqual(next.terminalSize.cols, 120);
    });
  });

  describe("global actions", () => {
    it("QUIT transitions to quit from any view", () => {
      const views = [
        "main_menu",
        "create_todo",
        "load_todo",
        "view_todo",
      ] as const;
      for (const view of views) {
        const state = { ...createInitialState(), view };
        const next = transition(state, { type: "QUIT" });
        assert.strictEqual(next.view, "quit");
      }
    });
  });
});
