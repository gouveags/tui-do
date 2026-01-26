import assert from "node:assert";
import { describe, it } from "node:test";
import type { TodoIndexEntry } from "../../src/state/types.ts";
import {
  dateFilterLabel,
  filterByDate,
  filterTodos,
  fuzzyMatch,
  fuzzyScore,
  nextDateFilter,
} from "../../src/utils/search.ts";

describe("fuzzyMatch", () => {
  it("matches exact string", () => {
    assert.strictEqual(fuzzyMatch("hello", "hello"), true);
  });

  it("matches subsequence", () => {
    assert.strictEqual(fuzzyMatch("hlo", "hello"), true);
  });

  it("matches case-insensitive", () => {
    assert.strictEqual(fuzzyMatch("HLO", "hello"), true);
    assert.strictEqual(fuzzyMatch("hlo", "HELLO"), true);
  });

  it("returns false when no match", () => {
    assert.strictEqual(fuzzyMatch("xyz", "hello"), false);
  });

  it("matches empty query to any string", () => {
    assert.strictEqual(fuzzyMatch("", "hello"), true);
  });

  it("handles multi-word queries", () => {
    assert.strictEqual(fuzzyMatch("gt", "grocery todo"), true);
  });
});

describe("fuzzyScore", () => {
  it("returns higher score for consecutive matches", () => {
    const score1 = fuzzyScore("hel", "hello");
    const score2 = fuzzyScore("hlo", "hello");
    assert.ok(score1 > score2);
  });

  it("returns higher score for start matches", () => {
    const score1 = fuzzyScore("he", "hello world");
    const score2 = fuzzyScore("wo", "hello world");
    assert.ok(score1 > score2);
  });

  it("returns -1 for no match", () => {
    assert.strictEqual(fuzzyScore("xyz", "hello"), -1);
  });

  it("returns 0 for empty query", () => {
    assert.strictEqual(fuzzyScore("", "hello"), 0);
  });
});

describe("filterTodos", () => {
  const entries: TodoIndexEntry[] = [
    {
      id: "1",
      title: "Grocery shopping",
      createdAt: 1000,
      updatedAt: 1000,
      itemCount: 5,
      doneCount: 2,
    },
    {
      id: "2",
      title: "Work tasks",
      createdAt: 2000,
      updatedAt: 2000,
      itemCount: 3,
      doneCount: 0,
    },
    {
      id: "3",
      title: "Home cleanup",
      createdAt: 3000,
      updatedAt: 3000,
      itemCount: 10,
      doneCount: 5,
    },
  ];

  it("returns all entries for empty query", () => {
    const result = filterTodos(entries, "");
    assert.strictEqual(result.length, 3);
  });

  it("filters by fuzzy match", () => {
    const result = filterTodos(entries, "work");
    assert.strictEqual(result.length, 1);
    assert.strictEqual(result[0]?.id, "2");
  });

  it("returns multiple matches sorted by score", () => {
    const result = filterTodos(entries, "o");
    assert.ok(result.length >= 2);
  });

  it("returns empty array for no matches", () => {
    const result = filterTodos(entries, "xyz123");
    assert.strictEqual(result.length, 0);
  });
});

describe("filterByDate", () => {
  const now = Date.now();
  const oneDay = 24 * 60 * 60 * 1000;
  const entries: TodoIndexEntry[] = [
    {
      id: "1",
      title: "Today",
      createdAt: now,
      updatedAt: now,
      itemCount: 0,
      doneCount: 0,
    },
    {
      id: "2",
      title: "Yesterday",
      createdAt: now - oneDay,
      updatedAt: now - oneDay,
      itemCount: 0,
      doneCount: 0,
    },
    {
      id: "3",
      title: "Last Month",
      createdAt: now - 40 * oneDay,
      updatedAt: now - 40 * oneDay,
      itemCount: 0,
      doneCount: 0,
    },
  ];

  it("returns all entries for 'all' filter", () => {
    const result = filterByDate(entries, "all");
    assert.strictEqual(result.length, 3);
  });

  it("filters by today", () => {
    const result = filterByDate(entries, "today");
    assert.strictEqual(result.length, 1);
    assert.strictEqual(result[0]?.title, "Today");
  });

  it("filters by week", () => {
    const result = filterByDate(entries, "week");
    assert.ok(result.length >= 1);
    assert.ok(result.some((e) => e.title === "Today"));
  });

  it("filters by month", () => {
    const result = filterByDate(entries, "month");
    assert.ok(result.length >= 1);
    assert.ok(result.length <= 2);
  });
});

describe("nextDateFilter", () => {
  it("cycles through filters", () => {
    assert.strictEqual(nextDateFilter("all"), "today");
    assert.strictEqual(nextDateFilter("today"), "week");
    assert.strictEqual(nextDateFilter("week"), "month");
    assert.strictEqual(nextDateFilter("month"), "all");
  });
});

describe("dateFilterLabel", () => {
  it("returns correct labels", () => {
    assert.strictEqual(dateFilterLabel("all"), "All");
    assert.strictEqual(dateFilterLabel("today"), "Today");
    assert.strictEqual(dateFilterLabel("week"), "This Week");
    assert.strictEqual(dateFilterLabel("month"), "This Month");
  });
});
