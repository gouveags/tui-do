import assert from "node:assert";
import { describe, it } from "node:test";
import { renderMarkdown } from "../../src/markdown/renderer.ts";
import { style } from "../../src/render/ansi.ts";

describe("markdown renderer", () => {
  it("renders plain text unchanged", () => {
    const result = renderMarkdown("hello world");
    assert.ok(result.includes("hello world"));
  });

  it("renders bold text with bold style", () => {
    const result = renderMarkdown("**bold**");
    assert.ok(result.includes(style.bold));
  });

  it("renders italic text with dim style", () => {
    const result = renderMarkdown("*italic*");
    assert.ok(result.includes(style.dim));
  });

  it("renders inline code with inverse style", () => {
    const result = renderMarkdown("`code`");
    assert.ok(result.includes(style.inverse));
  });

  it("renders h1 with cyan and bold", () => {
    const result = renderMarkdown("# Header");
    assert.ok(result.includes(style.bold));
    assert.ok(result.includes(style.fg.cyan));
  });

  it("renders bullet points with cyan bullet", () => {
    const result = renderMarkdown("- item");
    assert.ok(result.includes(style.fg.cyan));
    assert.ok(result.includes("•"));
  });

  it("renders quotes with dim style", () => {
    const result = renderMarkdown("> quote");
    assert.ok(result.includes(style.dim));
  });

  it("renders horizontal rule", () => {
    const result = renderMarkdown("---");
    assert.ok(result.includes("─"));
  });

  it("handles multiple lines", () => {
    const result = renderMarkdown("# Title\n\n- item");
    assert.ok(result.includes("Title"));
    assert.ok(result.includes("•"));
  });
});
