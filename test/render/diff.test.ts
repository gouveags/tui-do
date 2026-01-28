import assert from "node:assert";
import { describe, it } from "node:test";
import { diffGrids } from "../../src/render/diff.ts";
import { createGrid, setCell } from "../../src/render/grid.ts";

describe("diffGrids", () => {
  it("emits cursor moves and text for changed cells", () => {
    const prev = createGrid(3, 1);
    const next = createGrid(3, 1);
    setCell(next, 0, 0, { ch: "A", style: "" });

    const output = diffGrids(prev, next);
    assert.ok(output.includes("\x1b[1;1H"));
    assert.ok(output.includes("A"));
  });

  it("groups contiguous cells with same style", () => {
    const prev = createGrid(3, 1);
    const next = createGrid(3, 1);
    setCell(next, 0, 0, { ch: "A", style: "\x1b[32m" });
    setCell(next, 0, 1, { ch: "B", style: "\x1b[32m" });

    const output = diffGrids(prev, next);
    assert.ok(output.includes("\x1b[1;1H"));
    assert.ok(output.includes("AB"));
    assert.ok(!output.includes("\x1b[1;2H"));
  });

  it("splits runs when styles differ", () => {
    const prev = createGrid(3, 1);
    const next = createGrid(3, 1);
    setCell(next, 0, 0, { ch: "A", style: "\x1b[31m" });
    setCell(next, 0, 1, { ch: "B", style: "\x1b[34m" });

    const output = diffGrids(prev, next);
    let moves = 0;
    for (let i = 0; i < output.length; i++) {
      if (output[i] !== "\u001b" || output[i + 1] !== "[") continue;
      let j = i + 2;
      let hasFirst = false;
      while (j < output.length && output[j] >= "0" && output[j] <= "9") {
        hasFirst = true;
        j += 1;
      }
      if (!hasFirst || output[j] !== ";") continue;
      j += 1;
      let hasSecond = false;
      while (j < output.length && output[j] >= "0" && output[j] <= "9") {
        hasSecond = true;
        j += 1;
      }
      if (hasSecond && output[j] === "H") {
        moves += 1;
        i = j;
      }
    }
    assert.strictEqual(moves, 2);
  });
});
