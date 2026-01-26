export type LayoutRegion = {
  row: number;
  col: number;
  width: number;
  height: number;
};

export type AppLayout = {
  header: LayoutRegion;
  content: LayoutRegion;
  input: LayoutRegion;
  statusBar: LayoutRegion;
};

export type LayoutConfig = {
  maxWidth: number;
  minWidth: number;
  headerHeight: number;
  statusBarHeight: number;
  inputHeight: number;
};

export const DEFAULT_LAYOUT_CONFIG: LayoutConfig = {
  maxWidth: 120,
  minWidth: 40,
  headerHeight: 3,
  statusBarHeight: 1,
  inputHeight: 3,
};

export const calculateLayout = (
  terminalCols: number,
  terminalRows: number,
  config: LayoutConfig = DEFAULT_LAYOUT_CONFIG,
): AppLayout => {
  const effectiveWidth = Math.max(
    config.minWidth,
    Math.min(terminalCols - 4, config.maxWidth),
  );

  const startCol =
    terminalCols > config.maxWidth + 4
      ? Math.floor((terminalCols - effectiveWidth) / 2)
      : 2;

  const { headerHeight, inputHeight, statusBarHeight } = config;
  const fixedHeight = headerHeight + inputHeight + statusBarHeight;
  const contentHeight = Math.max(1, terminalRows - fixedHeight);

  const headerRow = 1;
  const contentRow = headerRow + headerHeight;
  const inputRow = terminalRows - statusBarHeight - inputHeight;
  const statusBarRow = terminalRows;

  return {
    header: {
      row: headerRow,
      col: startCol,
      width: effectiveWidth,
      height: headerHeight,
    },
    content: {
      row: contentRow,
      col: startCol,
      width: effectiveWidth,
      height: contentHeight,
    },
    input: {
      row: inputRow,
      col: startCol,
      width: effectiveWidth,
      height: inputHeight,
    },
    statusBar: {
      row: statusBarRow,
      col: startCol,
      width: effectiveWidth,
      height: statusBarHeight,
    },
  };
};

export const getTerminalSize = (): { rows: number; cols: number } => ({
  rows: process.stdout.rows || 24,
  cols: process.stdout.columns || 80,
});
