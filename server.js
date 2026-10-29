const http = require("http");
const fs = require("fs");
const path = require("path");
const { execFile } = require("child_process");

const PORT = Number(process.env.PORT) || 3000;
const HOST = process.env.HOST || "127.0.0.1";
const folder = __dirname;
const bridgePath = path.join(folder, "bridge.exe");
const routers = ["R1", "R2", "R3", "R4", "R5", "R6"];
const failedRouters = new Set();

function sendJSON(res, statusCode, payload) {
    res.writeHead(statusCode, {
        "Content-Type": "application/json; charset=utf-8",
        "Cache-Control": "no-store"
    });
    res.end(JSON.stringify(payload));
}

function runBridge(args) {
    return new Promise((resolve, reject) => {
        execFile(bridgePath, args, { windowsHide: true }, (error, stdout, stderr) => {
            if (error) {
                reject(new Error(stderr.trim() || error.message));
                return;
            }

            try {
                resolve(JSON.parse(stdout));
            } catch {
                reject(new Error("The network engine returned an invalid response."));
            }
        });
    });
}

function withRouterState(result) {
    return {
        ...result,
        routers,
        failedRouters: [...failedRouters]
    };
}

const server = http.createServer(async (req, res) => {
    let url;
    try {
        url = new URL(req.url, `http://${HOST}:${PORT}`);
    } catch {
        sendJSON(res, 400, { ok: false, error: "Invalid request URL" });
        return;
    }

    if (url.pathname.startsWith("/api/")) {
        if (url.pathname === "/api/status" && req.method === "GET") {
            try {
                const result = await runBridge(["status"]);
                if (!result.ok) {
                    sendJSON(res, 502, result);
                    return;
                }
                sendJSON(res, 200, withRouterState(result));
            } catch (error) {
                sendJSON(res, 503, { ok: false, error: error.message });
            }
            return;
        }

        if (url.pathname === "/api/route" && req.method === "GET") {
            const algorithm = url.searchParams.get("algorithm");
            const source = url.searchParams.get("source");
            const destination = url.searchParams.get("destination");

            if (
                !["bfs", "dijkstra"].includes(algorithm) ||
                !routers.includes(source) ||
                !routers.includes(destination)
            ) {
                sendJSON(res, 400, { ok: false, error: "Invalid algorithm or router" });
                return;
            }

            try {
                const result = await runBridge([
                    algorithm,
                    source,
                    destination,
                    ...failedRouters
                ]);
                if (!result.ok) {
                    sendJSON(res, 422, withRouterState(result));
                    return;
                }
                sendJSON(res, 200, withRouterState(result));
            } catch (error) {
                sendJSON(res, 503, { ok: false, error: error.message });
            }
            return;
        }

        if (url.pathname === "/api/router/fail" || url.pathname === "/api/router/recover") {
            if (req.method !== "POST") {
                sendJSON(res, 405, { ok: false, error: "Use POST for router operations" });
                return;
            }

            const router = url.searchParams.get("router");
            if (!routers.includes(router)) {
                sendJSON(res, 400, { ok: false, error: "Router must be R1 to R6" });
                return;
            }

            const failed = url.pathname.endsWith("/fail");
            if (failed) {
                failedRouters.add(router);
            } else {
                failedRouters.delete(router);
            }

            sendJSON(res, 200, withRouterState({
                ok: true,
                message: failed
                    ? `Router ${router} failure simulated.`
                    : `Router ${router} recovered successfully.`
            }));
            return;
        }

        if (url.pathname === "/api/reset" && req.method === "POST") {
            failedRouters.clear();
            sendJSON(res, 200, withRouterState({
                ok: true,
                message: "All routers restored."
            }));
            return;
        }

        sendJSON(res, 404, { ok: false, error: "API endpoint not found" });
        return;
    }

    if (req.method !== "GET") {
        sendJSON(res, 405, { ok: false, error: "Method not allowed" });
        return;
    }

    if (url.pathname === "/" || url.pathname === "/index.html") {
        fs.createReadStream(path.join(folder, "index.html"))
            .on("error", () => {
                if (!res.headersSent) {
                    sendJSON(res, 500, { ok: false, error: "Could not load the dashboard" });
                } else {
                    res.destroy();
                }
            })
            .once("open", () => {
                res.writeHead(200, {
                    "Content-Type": "text/html; charset=utf-8",
                    "Cache-Control": "no-store"
                });
            })
            .pipe(res);
        return;
    }

    if (url.pathname === "/favicon.ico") {
        res.writeHead(204);
        res.end();
        return;
    }

    sendJSON(res, 404, { ok: false, error: "File not found" });
});

server.listen(PORT, HOST, () => {
    console.log(`NetShield running at http://${HOST}:${PORT}`);
});
