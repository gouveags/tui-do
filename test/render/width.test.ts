import assert from "node:assert";
import { describe, it } from "node:test";
import { charWidth } from "../../src/render/width.ts";

describe("charWidth", () => {
  it("returns 1 for ASCII", () => {
    assert.strictEqual(charWidth("a"), 1);
  });

  it("returns 2 for wide characters", () => {
    assert.strictEqual(charWidth("界"), 2);
    assert.strictEqual(charWidth("😀"), 2);
  });

  it("returns 0 for combining marks", () => {
    assert.strictEqual(charWidth("\u0301"), 0);
  });
});
