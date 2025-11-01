import express from "express";
import http from "http";
import { WebSocketServer } from "ws";
import cors from "cors";
import { v4 as uuid } from "uuid";

const PORT = process.env.PORT || 8080;
const app = express();
app.use(cors());
app.get("/", (req, res) => res.send("ChatCloud server OK"));
app.get("/healthz", (req, res) => res.json({ ok: true }));

const server = http.createServer(app);
const wss = new WebSocketServer({ noServer: true });

const rooms = new Map();
const meta = new Map();

function broadcast(room, obj) {
  const set = rooms.get(room);
  if (!set) return;
  const data = JSON.stringify(obj);
  for (const ws of set) {
    if (ws.readyState === ws.OPEN) ws.send(data);
  }
}

function participants(room) {
  const set = rooms.get(room) || new Set();
  return Array.from(set).map(ws => meta.get(ws)?.user).filter(Boolean);
}

server.on("upgrade", (req, socket, head) => {
  const url = new URL(req.url, `http://${req.headers.host}`);
  if (url.pathname !== "/ws") { socket.destroy(); return; }
  const room = url.searchParams.get("room") || "global";
  const user = url.searchParams.get("user") || "anon";

  wss.handleUpgrade(req, socket, head, (ws) => {
    wss.emit("connection", ws, req, { room, user });
  });
});

wss.on("connection", (ws, _req, { room, user }) => {
  if (!rooms.has(room)) rooms.set(room, new Set());
  rooms.get(room).add(ws);
  meta.set(ws, { room, user });

  ws.send(JSON.stringify({ type: "participants", room, users: participants(room) }));
  broadcast(room, { type: "join", room, sender: user });

  ws.on("message", (data) => {
    try {
      const msg = JSON.parse(data.toString());
      if (msg.type === "message") {
        const payload = {
          type: "message",
          id: uuid(),
          room: meta.get(ws).room,
          sender: meta.get(ws).user,
          text: String(msg.text).slice(0, 2000),
          timestamp: Date.now()
        };
        broadcast(payload.room, payload);
      }
    } catch {}
  });

  ws.on("close", () => {
    const { room, user } = meta.get(ws) || {};
    if (rooms.has(room)) {
      rooms.get(room).delete(ws);
      if (rooms.get(room).size === 0) rooms.delete(room);
    }
    meta.delete(ws);
    if (room && user) broadcast(room, { type: "leave", room, sender: user });
  });
});

server.listen(PORT, () => console.log(`ChatCloud server on :${PORT}`));
