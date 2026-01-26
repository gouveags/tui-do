export type RenderBuffer = {
  readonly capacity: number;
  length: number;
  write: (str: string) => void;
  clear: () => void;
  toString: () => string;
  toBuffer: () => Buffer;
};

export const createBuffer = (initialCapacity: number): RenderBuffer => {
  let buf = Buffer.allocUnsafe(initialCapacity);
  let len = 0;

  const ensureCapacity = (needed: number) => {
    if (needed > buf.length) {
      const newCapacity = Math.max(buf.length * 2, needed);
      const newBuf = Buffer.allocUnsafe(newCapacity);
      buf.copy(newBuf, 0, 0, len);
      buf = newBuf;
    }
  };

  return {
    get capacity() {
      return buf.length;
    },
    get length() {
      return len;
    },
    set length(val: number) {
      len = val;
    },
    write(str: string) {
      const byteLen = Buffer.byteLength(str);
      ensureCapacity(len + byteLen);
      len += buf.write(str, len);
    },
    clear() {
      len = 0;
    },
    toString() {
      return buf.toString("utf8", 0, len);
    },
    toBuffer() {
      return buf.subarray(0, len);
    },
  };
};
