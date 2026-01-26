import assert from "node:assert";
import { describe, it } from "node:test";
import { calculateLayout, type LayoutConfig } from "../../src/render/layout.ts";

describe("layout calculations", () => {
  const defaultConfig: LayoutConfig = {
    maxWidth: 120,
    minWidth: 40,
    headerHeight: 3,
    statusBarHeight: 1,
    inputHeight: 3,
  };

  describe("small terminal", () => {
    it("uses full width on narrow terminal", () => {
      const layout = calculateLayout(80, 24, defaultConfig);
      assert.strictEqual(layout.content.width, 76);
      assert.strictEqual(layout.content.col, 2);
    });

    it("respects minimum width", () => {
      const layout = calculateLayout(30, 24, defaultConfig);
      assert.strictEqual(layout.content.width, 40);
    });
  });

  describe("large terminal", () => {
    it("centers content on wide terminal", () => {
      const layout = calculateLayout(160, 24, defaultConfig);
      assert.strictEqual(layout.content.width, 120);
      assert.ok(layout.content.col > 2);
    });

    it("calculates centered column correctly", () => {
      const layout = calculateLayout(160, 24, defaultConfig);
      const expectedCol = Math.floor((160 - 120) / 2);
      assert.strictEqual(layout.content.col, expectedCol);
    });
  });

  describe("vertical layout", () => {
    it("calculates header position", () => {
      const layout = calculateLayout(80, 24, defaultConfig);
      assert.strictEqual(layout.header.row, 1);
      assert.strictEqual(layout.header.height, 3);
    });

    it("calculates content area position", () => {
      const layout = calculateLayout(80, 24, defaultConfig);
      assert.strictEqual(layout.content.row, 4);
    });

    it("calculates input area position", () => {
      const layout = calculateLayout(80, 24, defaultConfig);
      const expectedRow =
        24 - defaultConfig.statusBarHeight - defaultConfig.inputHeight;
      assert.strictEqual(layout.input.row, expectedRow);
    });

    it("calculates status bar position", () => {
      const layout = calculateLayout(80, 24, defaultConfig);
      assert.strictEqual(layout.statusBar.row, 24);
    });

    it("calculates content height correctly", () => {
      const layout = calculateLayout(80, 24, defaultConfig);
      const totalFixed =
        defaultConfig.headerHeight +
        defaultConfig.inputHeight +
        defaultConfig.statusBarHeight;
      const expectedHeight = 24 - totalFixed;
      assert.strictEqual(layout.content.height, expectedHeight);
    });
  });

  describe("edge cases", () => {
    it("handles very small terminal", () => {
      const layout = calculateLayout(20, 10, defaultConfig);
      assert.ok(layout.content.height > 0);
      assert.strictEqual(layout.content.width, 40);
    });

    it("maintains consistent widths across regions", () => {
      const layout = calculateLayout(100, 30, defaultConfig);
      assert.strictEqual(layout.header.width, layout.content.width);
      assert.strictEqual(layout.content.width, layout.input.width);
      assert.strictEqual(layout.input.width, layout.statusBar.width);
    });
  });
});
