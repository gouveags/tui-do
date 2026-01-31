import fs from "node:fs";
import path from "node:path";

const distIndex = path.resolve("dist", "index.js");
const shebang = "#!/usr/bin/env node\n";

if (!fs.existsSync(distIndex)) {
  throw new Error(`Missing build output: ${distIndex}`);
}

const content = fs.readFileSync(distIndex, "utf8");
if (!content.startsWith(shebang)) {
  fs.writeFileSync(distIndex, shebang + content);
}
