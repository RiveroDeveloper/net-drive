const fs = require("fs");
const path = require("path");

const wsUrl = process.env.NETRIDE_WS_URL || "";

const content = `window.__NETRIDE_CONFIG__ = {
  WS_URL: ${JSON.stringify(wsUrl)},
};
`;

const outPath = path.join(__dirname, "..", "env.js");
fs.writeFileSync(outPath, content, "utf8");

console.log(`[build] Generated env.js with WS_URL=${wsUrl || "(empty)"}`);
