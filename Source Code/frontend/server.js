const http = require("node:http");
const fs = require("node:fs");
const path = require("node:path");

const root = path.resolve(__dirname);
const mimeTypes = {
  ".css": "text/css; charset=utf-8",
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".svg": "image/svg+xml"
};

http.createServer((request, response) => {
  const pathname = new URL(request.url, "http://localhost").pathname;
  let decodedPath;
  try {
    decodedPath = decodeURIComponent(pathname);
  } catch {
    response.writeHead(400).end("Invalid URL");
    return;
  }

  const requestedPath = decodedPath === "/" ? "/index.html" : decodedPath;
  const filePath = path.resolve(root, `.${requestedPath}`);
  if (!filePath.startsWith(`${root}${path.sep}`)) {
    response.writeHead(403).end("Forbidden");
    return;
  }

  fs.readFile(filePath, (error, content) => {
    if (error) {
      response.writeHead(error.code === "ENOENT" ? 404 : 500).end("File not found");
      return;
    }
    response.writeHead(200, {
      "Content-Type": mimeTypes[path.extname(filePath)] || "application/octet-stream",
      "X-Content-Type-Options": "nosniff",
      "Cache-Control": "no-store"
    });
    response.end(content);
  });
}).listen(3000, "127.0.0.1", () => {
  console.log("Vehicle Rental frontend is running at http://localhost:3000");
});
