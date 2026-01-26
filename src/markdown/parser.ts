export type TokenType =
  | "text"
  | "bold"
  | "italic"
  | "code"
  | "h1"
  | "h2"
  | "h3"
  | "bullet"
  | "numbered"
  | "quote"
  | "hr"
  | "codeblock"
  | "newline";

export type Token = {
  type: TokenType;
  content: string;
  number?: number;
};

const BLOCK_PATTERNS: Array<{ regex: RegExp; type: TokenType; group: number }> =
  [
    { regex: /^### (.+)$/, type: "h3", group: 1 },
    { regex: /^## (.+)$/, type: "h2", group: 1 },
    { regex: /^# (.+)$/, type: "h1", group: 1 },
    { regex: /^[-*] (.+)$/, type: "bullet", group: 1 },
    { regex: /^(\d+)\. (.+)$/, type: "numbered", group: 2 },
    { regex: /^> (.+)$/, type: "quote", group: 1 },
    { regex: /^---+$/, type: "hr", group: 0 },
  ];

const tokenizeLine = (line: string): Token[] => {
  const trimmed = line.trim();

  if (!trimmed) {
    return [{ type: "newline", content: "" }];
  }

  for (const { regex, type, group } of BLOCK_PATTERNS) {
    const match = trimmed.match(regex);
    if (match) {
      const content = group === 0 ? "" : (match[group] ?? "");
      if (type === "numbered") {
        return [
          { type, content, number: Number.parseInt(match[1] ?? "1", 10) },
        ];
      }
      return [{ type, content }];
    }
  }

  return tokenizeInline(trimmed);
};

const tokenizeInline = (text: string): Token[] => {
  const tokens: Token[] = [];
  let remaining = text;

  const patterns = [
    { regex: /\*\*(.+?)\*\*/, type: "bold" as TokenType },
    { regex: /\*(.+?)\*/, type: "italic" as TokenType },
    { regex: /`(.+?)`/, type: "code" as TokenType },
  ];

  while (remaining) {
    let earliestMatch: {
      index: number;
      length: number;
      type: TokenType;
      content: string;
    } | null = null;

    for (const { regex, type } of patterns) {
      const match = remaining.match(regex);
      if (match && match.index !== undefined) {
        if (!earliestMatch || match.index < earliestMatch.index) {
          earliestMatch = {
            index: match.index,
            length: match[0].length,
            type,
            content: match[1] ?? "",
          };
        }
      }
    }

    if (earliestMatch) {
      if (earliestMatch.index > 0) {
        tokens.push({
          type: "text",
          content: remaining.slice(0, earliestMatch.index),
        });
      }
      tokens.push({ type: earliestMatch.type, content: earliestMatch.content });
      remaining = remaining.slice(earliestMatch.index + earliestMatch.length);
    } else {
      if (remaining) {
        tokens.push({ type: "text", content: remaining });
      }
      break;
    }
  }

  return tokens;
};

export const tokenize = (input: string): Token[] => {
  const lines = input.split("\n");
  const tokens: Token[] = [];
  let inCodeBlock = false;
  let codeBlockContent: string[] = [];

  for (const line of lines) {
    if (line.trim().startsWith("```")) {
      if (inCodeBlock) {
        tokens.push({
          type: "codeblock",
          content: codeBlockContent.join("\n"),
        });
        codeBlockContent = [];
        inCodeBlock = false;
      } else {
        inCodeBlock = true;
      }
      continue;
    }

    if (inCodeBlock) {
      codeBlockContent.push(line);
      continue;
    }

    tokens.push(...tokenizeLine(line));
  }

  if (inCodeBlock && codeBlockContent.length > 0) {
    tokens.push({ type: "codeblock", content: codeBlockContent.join("\n") });
  }

  return tokens;
};

export const parseInline = tokenizeInline;
