import assert from "node:assert";
import { beforeEach, describe, it } from "node:test";
import type { RenderBuffer } from "../../src/render/buffer.ts";
import { createBuffer } from "../../src/render/buffer.ts";

describe("RenderBuffer", () => {
  let buffer: RenderBuffer;

  beforeEach(() => {
    buffer = createBuffer(1024);
  });

  it("creates buffer with specified capacity", () => {
    assert.strictEqual(buffer.capacity, 1024);
  });

  it("starts with zero length", () => {
    assert.strictEqual(buffer.length, 0);
  });

  it("writes string and updates length", () => {
    buffer.write("hello");
    assert.strictEqual(buffer.length, 5);
  });

  it("writes multiple strings", () => {
    buffer.write("hello");
    buffer.write(" world");
    assert.strictEqual(buffer.length, 11);
  });

  it("clears buffer", () => {
    buffer.write("hello");
    buffer.clear();
    assert.strictEqual(buffer.length, 0);
  });

  it("returns string from toString()", () => {
    buffer.write("hello");
    buffer.write(" world");
    assert.strictEqual(buffer.toString(), "hello world");
  });

  it("returns empty string when empty", () => {
    assert.strictEqual(buffer.toString(), "");
  });

  it("returns correct buffer slice", () => {
    buffer.write("hello");
    const slice = buffer.toBuffer();
    assert.strictEqual(slice.length, 5);
    assert.strictEqual(slice.toString(), "hello");
  });

  it("handles unicode characters", () => {
    buffer.write("┌─┐");
    assert.strictEqual(buffer.toString(), "┌─┐");
  });

  it("handles emoji", () => {
    buffer.write("✔ done");
    assert.strictEqual(buffer.toString(), "✔ done");
  });

  it("grows buffer when needed", () => {
    const smallBuffer = createBuffer(8);
    smallBuffer.write("this is a longer string");
    assert.strictEqual(smallBuffer.toString(), "this is a longer string");
    assert.ok(smallBuffer.capacity >= 23);
  });
});
