/*const express = require("express");
const app = express();
const port = 3000;

app.use(express.json());

// メモリ上にサーバー情報を保持
let servers = [];

// サーバー登録
app.post("/register", (req, res) => {
  const { ip, port, name, map } = req.body;
  servers.push({ ip, port, name, map, time: Date.now() });
  console.log(`Registered: ${name} (${ip}:${port})`);
  res.sendStatus(200);
});

// サーバー一覧を返す
app.get("/list", (req, res) => {
  // 30秒以上更新がないサーバーは削除
  const now = Date.now();
  servers = servers.filter(s => now - s.time < 30000);
  res.json(servers);
});

// サーバー削除（オプション）
app.post("/unregister", (req, res) => {
  const { ip, port } = req.body;
  servers = servers.filter(s => !(s.ip === ip && s.port === port));
  console.log(`Unregistered: ${ip}:${port}`);
  res.sendStatus(200);
});

// 起動
app.listen(port, () => {
  console.log(`✅ Master Server running at http://localhost:${port}`);
});*/

/*const express = require("express");
const app = express();
const PORT = 3000;

app.get("/servers", (req, res) => {
  res.json({
    servers: [
      { name: "Test Server 1", address: "127.0.0.1", players: 3 },
      { name: "Test Server 2", address: "192.168.0.5", players: 5 },
    ],
  });
});

app.listen(PORT, () => console.log(`Master server running on port ${PORT}`));
*/

const express = require('express');
const app = express();
app.use(express.json());

let servers = [];

app.post('/register', (req, res) => {
  const { name, address, playerCount, maxPlayers } = req.body;
  const existing = servers.find(s => s.address === address);
  if (!existing) {
    servers.push({ name, address, playerCount, maxPlayers, time: Date.now() });
  } else {
    existing.name = name;
    existing.playerCount = playerCount;
    existing.maxPlayers = maxPlayers;
    existing.time = Date.now();
  }
  res.send({ success: true });
});

app.get('/servers', (req, res) => {
  // optional: prune old entries
  const now = Date.now();
  servers = servers.filter(s => now - s.time < 5 * 60 * 1000);
  res.json({ servers });
});

app.put('/api/servers/update', (req, res) => {
    const { name, playerCount, maxPlayers } = req.body;
    const server = servers.find(s => s.name === name);

    if (server) {
        server.playerCount = playerCount;
        server.maxPlayers = maxPlayers;
        server.time = Date.now();
        res.json({ message: 'Updated', server });
    } else {
        res.status(404).json({ message: 'Server not found' });
    }
  });
  
app.delete('/unregister', (req, res) => {
  const { address } = req.body;
  servers = servers.filter(s => s.address !== address);
  res.send({ success: true });
});

app.listen(3000, "0.0.0.0", () => {
  console.log('Server is running on http://0.0.0.0:3000');});