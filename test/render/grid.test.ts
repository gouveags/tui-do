import assert from "node:assert";
import { describe, it } from "node:test";
import {
  clearGrid,
  createGrid,
  getCell,
  setCell,
} from "../../src/render/grid.ts";

describe("grid", () => {
  it("sets and gets cells", () => {
    const grid = createGrid(2, 2);
    setCell(grid, 0, 1, { ch: "A", style: "\x1b[31m" });

    const cell = getCell(grid, 0, 1);
    assert.strictEqual(cell.ch, "A");
    assert.strictEqual(cell.style, "\x1b[31m");
  });

  it("clears cells to defaults", () => {
    const grid = createGrid(1, 1);
    setCell(grid, 0, 0, { ch: "Z", style: "\x1b[31m" });

    clearGrid(grid);
    const cell = getCell(grid, 0, 0);
    assert.strictEqual(cell.ch, " ");
    assert.strictEqual(cell.style, "");
  });
});
