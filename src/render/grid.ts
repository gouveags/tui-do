export type Cell = {
  ch: string;
  style: string;
};

export type Grid = {
  width: number;
  height: number;
  cells: Cell[];
};

const createCell = (cell: Cell): Cell => ({ ch: cell.ch, style: cell.style });

export const createGrid = (
  width: number,
  height: number,
  fill?: Cell,
): Grid => {
  const fillCell = fill ?? { ch: " ", style: "" };
  const cells = Array.from({ length: width * height }, () =>
    createCell(fillCell),
  );
  return { width, height, cells };
};

export const clearGrid = (grid: Grid, fill?: Cell): void => {
  const fillCell = fill ?? { ch: " ", style: "" };
  for (const cell of grid.cells) {
    cell.ch = fillCell.ch;
    cell.style = fillCell.style;
  }
};

export const setCell = (
  grid: Grid,
  row: number,
  col: number,
  cell: Cell,
): void => {
  if (row < 0 || col < 0 || row >= grid.height || col >= grid.width) {
    return;
  }
  const idx = row * grid.width + col;
  grid.cells[idx].ch = cell.ch;
  grid.cells[idx].style = cell.style;
};

export const getCell = (grid: Grid, row: number, col: number): Cell => {
  const idx = row * grid.width + col;
  return grid.cells[idx];
};
