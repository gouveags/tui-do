import { cursor, style as styleCodes } from "./ansi.ts";
import { createBuffer } from "./buffer.ts";
import type { Grid } from "./grid.ts";

export const diffGrids = (prev: Grid, next: Grid): string => {
  if (prev.width !== next.width || prev.height !== next.height) {
    throw new Error("Grid sizes must match for diffing.");
  }

  const buf = createBuffer(4096);
  let hasChanges = false;
  let lastStyle = styleCodes.reset;

  for (let row = 0; row < next.height; row++) {
    let col = 0;
    while (col < next.width) {
      const idx = row * next.width + col;
      const prevCell = prev.cells[idx];
      const nextCell = next.cells[idx];
      const changed =
        prevCell.ch !== nextCell.ch || prevCell.style !== nextCell.style;

      if (!changed) {
        col += 1;
        continue;
      }

      const startCol = col;
      const runStyle = nextCell.style;
      let text = "";

      while (col < next.width) {
        const runIdx = row * next.width + col;
        const runPrev = prev.cells[runIdx];
        const runNext = next.cells[runIdx];
        const runChanged =
          runPrev.ch !== runNext.ch || runPrev.style !== runNext.style;
        if (!runChanged || runNext.style !== runStyle) {
          break;
        }
        text += runNext.ch;
        col += 1;
      }

      const nextStyle = runStyle.length > 0 ? runStyle : styleCodes.reset;
      buf.write(cursor.moveTo(row + 1, startCol + 1));
      if (nextStyle !== lastStyle) {
        buf.write(nextStyle);
        lastStyle = nextStyle;
      }
      buf.write(text);
      hasChanges = true;
    }
  }

  if (hasChanges && lastStyle !== styleCodes.reset) {
    buf.write(styleCodes.reset);
  }

  return buf.toString();
};
