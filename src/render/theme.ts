import { bg256, fg256, style } from "./ansi.ts";

export type ThemeColors = {
  title: string;
  selected: string;
  completed: string;
  help: string;
  border: string;
  cursor: string;
  error: string;
  bullet: string;
  quote: string;
  code: string;
  header1: string;
  header2: string;
  header3: string;
  accent: string;
  statusBar: string;
  statusBarAccent: string;
  panelBorder: string;
  panelTitle: string;
  hintKey: string;
  hintText: string;
  inputBorder: string;
  inputLabel: string;
  inputText: string;
};

export type Theme = {
  colors: ThemeColors;
  reset: string;
};

export const defaultTheme: Theme = {
  colors: {
    title: style.bold + fg256(81),
    selected: style.bold + fg256(231) + bg256(24),
    completed: fg256(70),
    help: fg256(244),
    border: fg256(238),
    cursor: style.inverse,
    error: style.bold + fg256(203),
    bullet: fg256(111),
    quote: fg256(245),
    code: fg256(229) + bg256(236),
    header1: style.bold + fg256(81),
    header2: style.bold + fg256(75),
    header3: style.bold + fg256(69),
    accent: fg256(141),
    statusBar: style.bold + fg256(231) + bg256(23),
    statusBarAccent: style.bold + fg256(86) + bg256(23),
    panelBorder: fg256(239),
    panelTitle: style.bold + fg256(117),
    hintKey: style.bold + fg256(229),
    hintText: fg256(246),
    inputBorder: fg256(60),
    inputLabel: style.bold + fg256(111),
    inputText: style.reset,
  },
  reset: style.reset,
};

export const applyStyle = (text: string, styleCode: string): string =>
  styleCode + text + style.reset;

export const styled = {
  title: (text: string) => applyStyle(text, defaultTheme.colors.title),
  selected: (text: string) => applyStyle(text, defaultTheme.colors.selected),
  completed: (text: string) => applyStyle(text, defaultTheme.colors.completed),
  help: (text: string) => applyStyle(text, defaultTheme.colors.help),
  border: (text: string) => applyStyle(text, defaultTheme.colors.border),
  error: (text: string) => applyStyle(text, defaultTheme.colors.error),
  bullet: (text: string) => applyStyle(text, defaultTheme.colors.bullet),
  code: (text: string) => applyStyle(text, defaultTheme.colors.code),
};
