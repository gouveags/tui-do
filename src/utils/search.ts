import type { DateFilter, TodoIndexEntry } from "../state/types.ts";

export const fuzzyMatch = (query: string, text: string): boolean => {
  const q = query.toLowerCase();
  const t = text.toLowerCase();
  let qi = 0;
  for (let ti = 0; ti < t.length && qi < q.length; ti++) {
    if (t[ti] === q[qi]) qi++;
  }
  return qi === q.length;
};

export const fuzzyScore = (query: string, text: string): number => {
  const q = query.toLowerCase();
  const t = text.toLowerCase();
  if (!q) return 0;

  let score = 0;
  let qi = 0;
  let consecutive = 0;

  for (let ti = 0; ti < t.length && qi < q.length; ti++) {
    if (t[ti] === q[qi]) {
      if (ti === qi) score += 10;
      consecutive++;
      score += consecutive;
      qi++;
    } else {
      consecutive = 0;
    }
  }

  if (qi < q.length) return -1;
  if (t.startsWith(q)) score += 20;
  return score;
};

export const fuzzyMatchIndices = (query: string, text: string): number[] => {
  const q = query.toLowerCase().trim();
  if (!q) return [];
  const t = text.toLowerCase();
  let qi = 0;
  const matches: number[] = [];

  for (let ti = 0; ti < t.length && qi < q.length; ti++) {
    if (t[ti] === q[qi]) {
      matches.push(ti);
      qi++;
    }
  }

  return qi === q.length ? matches : [];
};

export const filterTodos = (
  entries: TodoIndexEntry[],
  query: string,
): TodoIndexEntry[] => {
  if (!query.trim()) return entries;

  return entries
    .map((entry) => ({ entry, score: fuzzyScore(query, entry.title) }))
    .filter((item) => item.score >= 0)
    .sort((a, b) => b.score - a.score)
    .map((item) => item.entry);
};

const getStartOfDay = (date: Date): number => {
  const d = new Date(date);
  d.setHours(0, 0, 0, 0);
  return d.getTime();
};

const getStartOfWeek = (date: Date): number => {
  const d = new Date(date);
  const day = d.getDay();
  d.setDate(d.getDate() - day);
  d.setHours(0, 0, 0, 0);
  return d.getTime();
};

const getStartOfMonth = (date: Date): number => {
  const d = new Date(date);
  d.setDate(1);
  d.setHours(0, 0, 0, 0);
  return d.getTime();
};

export const filterByDate = (
  entries: TodoIndexEntry[],
  filter: DateFilter,
): TodoIndexEntry[] => {
  if (filter === "all") return entries;

  const now = new Date();
  let cutoff: number;

  switch (filter) {
    case "today":
      cutoff = getStartOfDay(now);
      break;
    case "week":
      cutoff = getStartOfWeek(now);
      break;
    case "month":
      cutoff = getStartOfMonth(now);
      break;
  }

  return entries.filter((e) => e.createdAt >= cutoff);
};

export const nextDateFilter = (current: DateFilter): DateFilter => {
  const filters: DateFilter[] = ["all", "today", "week", "month"];
  const idx = filters.indexOf(current);
  return filters[(idx + 1) % filters.length] ?? "all";
};

export const dateFilterLabel = (filter: DateFilter): string => {
  switch (filter) {
    case "all":
      return "All";
    case "today":
      return "Today";
    case "week":
      return "This Week";
    case "month":
      return "This Month";
  }
};
