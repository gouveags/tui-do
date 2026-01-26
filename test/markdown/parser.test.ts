import assert from "node:assert";
import { describe, it } from "node:test";
import { tokenize } from "../../src/markdown/parser.ts";

describe("markdown tokenizer", () => {
  describe("inline styles", () => {
    it("tokenizes bold text", () => {
      const tokens = tokenize("hello **world**");
      assert.ok(tokens.some((t) => t.type === "bold" && t.content === "world"));
    });

    it("tokenizes italic text", () => {
      const tokens = tokenize("hello *world*");
      assert.ok(
        tokens.some((t) => t.type === "italic" && t.content === "world"),
      );
    });

    it("tokenizes inline code", () => {
      const tokens = tokenize("use `const x = 1`");
      assert.ok(
        tokens.some((t) => t.type === "code" && t.content === "const x = 1"),
      );
    });

    it("handles multiple inline styles", () => {
      const tokens = tokenize("**bold** and *italic* and `code`");
      assert.ok(tokens.some((t) => t.type === "bold"));
      assert.ok(tokens.some((t) => t.type === "italic"));
      assert.ok(tokens.some((t) => t.type === "code"));
    });
  });

  describe("block elements", () => {
    it("tokenizes h1 header", () => {
      const tokens = tokenize("# Hello");
      assert.ok(tokens.some((t) => t.type === "h1" && t.content === "Hello"));
    });

    it("tokenizes h2 header", () => {
      const tokens = tokenize("## Hello");
      assert.ok(tokens.some((t) => t.type === "h2" && t.content === "Hello"));
    });

    it("tokenizes h3 header", () => {
      const tokens = tokenize("### Hello");
      assert.ok(tokens.some((t) => t.type === "h3" && t.content === "Hello"));
    });

    it("tokenizes bullet list with dash", () => {
      const tokens = tokenize("- item one");
      assert.ok(
        tokens.some((t) => t.type === "bullet" && t.content === "item one"),
      );
    });

    it("tokenizes bullet list with asterisk", () => {
      const tokens = tokenize("* item one");
      assert.ok(
        tokens.some((t) => t.type === "bullet" && t.content === "item one"),
      );
    });

    it("tokenizes numbered list", () => {
      const tokens = tokenize("1. first item");
      assert.ok(
        tokens.some((t) => t.type === "numbered" && t.content === "first item"),
      );
    });

    it("tokenizes quote", () => {
      const tokens = tokenize("> quoted text");
      assert.ok(
        tokens.some((t) => t.type === "quote" && t.content === "quoted text"),
      );
    });

    it("tokenizes horizontal rule", () => {
      const tokens = tokenize("---");
      assert.ok(tokens.some((t) => t.type === "hr"));
    });

    it("tokenizes code block", () => {
      const tokens = tokenize("```\ncode here\n```");
      assert.ok(tokens.some((t) => t.type === "codeblock"));
    });
  });

  describe("multiline", () => {
    it("handles multiple lines", () => {
      const input = "# Title\n\nSome text\n\n- item";
      const tokens = tokenize(input);
      assert.ok(tokens.some((t) => t.type === "h1"));
      assert.ok(tokens.some((t) => t.type === "text"));
      assert.ok(tokens.some((t) => t.type === "bullet"));
    });
  });

  describe("plain text", () => {
    it("returns text token for plain text", () => {
      const tokens = tokenize("hello world");
      assert.strictEqual(tokens.length, 1);
      assert.strictEqual(tokens[0]?.type, "text");
      assert.strictEqual(tokens[0]?.content, "hello world");
    });
  });
});
