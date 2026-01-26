import type { CursorPosition, InputBuffer } from "../state/types.ts";

export { createInputBuffer } from "../state/types.ts";

export const getLineText = (buf: InputBuffer): string =>
  buf.lines[buf.cursor.line] ?? "";

export const getTotalLength = (buf: InputBuffer): number =>
  buf.lines.reduce((sum, line, i) => sum + line.length + (i > 0 ? 1 : 0), 0);

const clampCol = (lines: string[], line: number, col: number): number =>
  Math.min(col, lines[line]?.length ?? 0);

const withCursor = (buf: InputBuffer, cursor: CursorPosition): InputBuffer => ({
  ...buf,
  cursor,
});

const withLines = (
  buf: InputBuffer,
  lines: string[],
  cursor?: CursorPosition,
): InputBuffer => ({
  lines,
  cursor: cursor ?? buf.cursor,
});

export const moveLeft = (buf: InputBuffer): InputBuffer => {
  const { line, col } = buf.cursor;
  if (col > 0) {
    return withCursor(buf, { line, col: col - 1 });
  }
  if (line > 0) {
    const prevLine = buf.lines[line - 1] ?? "";
    return withCursor(buf, { line: line - 1, col: prevLine.length });
  }
  return buf;
};

export const moveRight = (buf: InputBuffer): InputBuffer => {
  const { line, col } = buf.cursor;
  const currentLine = buf.lines[line] ?? "";
  if (col < currentLine.length) {
    return withCursor(buf, { line, col: col + 1 });
  }
  if (line < buf.lines.length - 1) {
    return withCursor(buf, { line: line + 1, col: 0 });
  }
  return buf;
};

export const moveUp = (buf: InputBuffer): InputBuffer => {
  const { line, col } = buf.cursor;
  if (line > 0) {
    return withCursor(buf, {
      line: line - 1,
      col: clampCol(buf.lines, line - 1, col),
    });
  }
  return buf;
};

export const moveDown = (buf: InputBuffer): InputBuffer => {
  const { line, col } = buf.cursor;
  if (line < buf.lines.length - 1) {
    return withCursor(buf, {
      line: line + 1,
      col: clampCol(buf.lines, line + 1, col),
    });
  }
  return buf;
};

export const moveToLineStart = (buf: InputBuffer): InputBuffer =>
  withCursor(buf, { line: buf.cursor.line, col: 0 });

export const moveToLineEnd = (buf: InputBuffer): InputBuffer => {
  const line = buf.cursor.line;
  return withCursor(buf, { line, col: buf.lines[line]?.length ?? 0 });
};

export const insertChar = (buf: InputBuffer, char: string): InputBuffer => {
  const { line, col } = buf.cursor;
  const currentLine = buf.lines[line] ?? "";
  const newLine = currentLine.slice(0, col) + char + currentLine.slice(col);
  const newLines = [...buf.lines];
  newLines[line] = newLine;
  return withLines(buf, newLines, { line, col: col + char.length });
};

export const deleteBackward = (buf: InputBuffer): InputBuffer => {
  const { line, col } = buf.cursor;

  if (col > 0) {
    const currentLine = buf.lines[line] ?? "";
    const newLine = currentLine.slice(0, col - 1) + currentLine.slice(col);
    const newLines = [...buf.lines];
    newLines[line] = newLine;
    return withLines(buf, newLines, { line, col: col - 1 });
  }

  if (line > 0) {
    const prevLine = buf.lines[line - 1] ?? "";
    const currentLine = buf.lines[line] ?? "";
    const newLines = [
      ...buf.lines.slice(0, line - 1),
      prevLine + currentLine,
      ...buf.lines.slice(line + 1),
    ];
    return withLines(buf, newLines, { line: line - 1, col: prevLine.length });
  }

  return buf;
};

export const deleteForward = (buf: InputBuffer): InputBuffer => {
  const { line, col } = buf.cursor;
  const currentLine = buf.lines[line] ?? "";

  if (col < currentLine.length) {
    const newLine = currentLine.slice(0, col) + currentLine.slice(col + 1);
    const newLines = [...buf.lines];
    newLines[line] = newLine;
    return withLines(buf, newLines, buf.cursor);
  }

  if (line < buf.lines.length - 1) {
    const nextLine = buf.lines[line + 1] ?? "";
    const newLines = [
      ...buf.lines.slice(0, line),
      currentLine + nextLine,
      ...buf.lines.slice(line + 2),
    ];
    return withLines(buf, newLines, buf.cursor);
  }

  return buf;
};

export const insertNewline = (buf: InputBuffer): InputBuffer => {
  const { line, col } = buf.cursor;
  const currentLine = buf.lines[line] ?? "";
  const before = currentLine.slice(0, col);
  const after = currentLine.slice(col);
  const newLines = [
    ...buf.lines.slice(0, line),
    before,
    after,
    ...buf.lines.slice(line + 1),
  ];
  return withLines(buf, newLines, { line: line + 1, col: 0 });
};

export const getBufferText = (buf: InputBuffer): string => buf.lines.join("\n");

export const setBufferText = (_buf: InputBuffer, text: string): InputBuffer => {
  const lines = text.split("\n");
  const lastLine = lines[lines.length - 1] ?? "";
  return {
    lines,
    cursor: { line: lines.length - 1, col: lastLine.length },
  };
};

export const clearBuffer = (): InputBuffer => ({
  lines: [""],
  cursor: { line: 0, col: 0 },
});
