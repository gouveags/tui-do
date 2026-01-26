import type { KeyEvent } from "./keys.ts";
import { parseKey } from "./keys.ts";

export type KeyHandler = (key: KeyEvent) => void;

let originalRawMode: boolean | undefined;

export const enableRawMode = (): void => {
  if (process.stdin.isTTY) {
    originalRawMode = process.stdin.isRaw;
    process.stdin.setRawMode(true);
  }
};

export const disableRawMode = (): void => {
  if (process.stdin.isTTY && originalRawMode !== undefined) {
    process.stdin.setRawMode(originalRawMode);
  }
};

export const startKeyListener = (handler: KeyHandler): (() => void) => {
  const onData = (data: Buffer) => {
    handler(parseKey(data));
  };

  process.stdin.on("data", onData);
  process.stdin.resume();

  return () => {
    process.stdin.off("data", onData);
    process.stdin.pause();
  };
};
