import assert from "node:assert";
import { describe, it } from "node:test";
import { formatDate } from "../../src/utils/date.ts";

describe("formatDate", () => {
  it("formats morning time correctly", () => {
    const timestamp = new Date(2026, 0, 25, 9, 5).getTime();
    assert.strictEqual(formatDate(timestamp), "Jan 25, 2026 9:05 AM");
  });

  it("formats afternoon time correctly", () => {
    const timestamp = new Date(2026, 0, 25, 15, 45).getTime();
    assert.strictEqual(formatDate(timestamp), "Jan 25, 2026 3:45 PM");
  });

  it("formats noon as 12 PM", () => {
    const timestamp = new Date(2026, 5, 15, 12, 0).getTime();
    assert.strictEqual(formatDate(timestamp), "Jun 15, 2026 12:00 PM");
  });

  it("formats midnight as 12 AM", () => {
    const timestamp = new Date(2026, 11, 31, 0, 30).getTime();
    assert.strictEqual(formatDate(timestamp), "Dec 31, 2026 12:30 AM");
  });

  it("pads minutes with leading zero", () => {
    const timestamp = new Date(2026, 2, 10, 14, 5).getTime();
    assert.strictEqual(formatDate(timestamp), "Mar 10, 2026 2:05 PM");
  });
});
