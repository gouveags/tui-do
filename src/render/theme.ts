import { style } from "./ansi.ts";

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
};

export type Theme = {
  colors: ThemeColors;
  reset: string;
};

export const defaultTheme: Theme = {
  colors: {
    title: style.bold + style.fg.cyan,
    selected: style.inverse,
    completed: style.fg.green,
    help: style.dim,
    border: style.dim,
    cursor: style.inverse,
    error: style.fg.red,
    bullet: style.fg.cyan,
    quote: style.dim,
    code: style.inverse,
    header1: style.bold + style.fg.cyan,
    header2: style.bold + style.fg.cyan,
    header3: style.bold + style.fg.cyan,
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
