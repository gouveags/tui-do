import { style } from "../render/ansi.ts";
import { defaultTheme } from "../render/theme.ts";
import { parseInline, type Token, tokenize } from "./parser.ts";

const BULLET_CHAR = "•";
const HR_CHAR = "─";
const INDENT = "  ";

const renderInlineToken = (token: Token): string => {
  switch (token.type) {
    case "bold":
      return style.bold + token.content + style.reset;
    case "italic":
      return style.dim + token.content + style.reset;
    case "code":
      return style.inverse + token.content + style.reset;
    case "text":
      return token.content;
    default:
      return token.content;
  }
};

const renderInlineContent = (content: string): string => {
  const tokens = parseInline(content);
  return tokens.map(renderInlineToken).join("");
};

const renderToken = (token: Token, width = 80): string => {
  switch (token.type) {
    case "h1":
      return (
        defaultTheme.colors.header1 +
        renderInlineContent(token.content) +
        style.reset
      );
    case "h2":
      return (
        defaultTheme.colors.header2 +
        renderInlineContent(token.content) +
        style.reset
      );
    case "h3":
      return (
        defaultTheme.colors.header3 +
        renderInlineContent(token.content) +
        style.reset
      );
    case "bullet":
      return (
        defaultTheme.colors.bullet +
        BULLET_CHAR +
        style.reset +
        " " +
        renderInlineContent(token.content)
      );
    case "numbered":
      return `${token.number ?? 1}. ${renderInlineContent(token.content)}`;
    case "quote":
      return (
        style.dim + INDENT + renderInlineContent(token.content) + style.reset
      );
    case "hr":
      return style.dim + HR_CHAR.repeat(Math.min(width, 40)) + style.reset;
    case "codeblock":
      return (
        style.dim +
        token.content
          .split("\n")
          .map((line) => INDENT + line)
          .join("\n") +
        style.reset
      );
    case "newline":
      return "";
    case "bold":
    case "italic":
    case "code":
    case "text":
      return renderInlineToken(token);
    default:
      return token.content;
  }
};

export const renderMarkdown = (input: string, width = 80): string => {
  const tokens = tokenize(input);
  const lines: string[] = [];

  for (const token of tokens) {
    if (token.type === "newline") {
      lines.push("");
    } else {
      lines.push(renderToken(token, width));
    }
  }

  return lines.join("\n");
};

export const renderMarkdownLines = (input: string, width = 80): string[] => {
  const tokens = tokenize(input);
  const lines: string[] = [];

  for (const token of tokens) {
    if (token.type === "newline") {
      lines.push("");
    } else if (token.type === "codeblock") {
      const rendered = renderToken(token, width);
      lines.push(...rendered.split("\n"));
    } else {
      lines.push(renderToken(token, width));
    }
  }

  return lines;
};
