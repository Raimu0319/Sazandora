const express = require('express');
const app = express();
app.use(express.json());

let servers = [];

app.post('/register', (req, res) => {
  const { name, address, playerCount, maxPlayers, gameplay} = req.body;
 
  const existing = servers.find(s => s.address === address);
  if (!existing) {
    servers.push({ name, address, playerCount, maxPlayers, gameplay, time: Date.now() });
  } else {
    existing.name = name;
    existing.playerCount = playerCount;
    existing.maxPlayers = maxPlayers;
    existing.gameplay = gameplay;
    existing.time = Date.now();
  }
  res.send({ success: true });
});

//サーバー情報取得
app.get('/servers', (req, res) => {
  // optional: prune old entries
  const now = Date.now();
  servers = servers.filter(s => now - s.time < 5 * 60 * 1000);
  res.json({ servers });
});

//サーバー情報更新
app.put('/api/servers/update', (req, res) => {
    const { name, playerCount, maxPlayers, gameplay} = req.body;
    const server = servers.find(s => s.name === name);

    if (server) 
    {
        server.playerCount = playerCount;
        server.maxPlayers = maxPlayers;
        server.gameplay = gameplay;
        server.time = Date.now();
        res.json({ message: 'Updated', server });
    } else {
        res.status(404).json({ message: 'Server not found' });
    }
  });


app.post('/shutdown', (req, res) => {
    console.log("🔻 Shutdown request received. Closing server...");

    res.json({ shutting_down: true });

    // 0.5秒遅らせて安全にコネクションを閉じる
    setTimeout(() => {
        console.log("🔻 Master Server is shutting down now.");
        process.exit(0);   // ← Node.js プロセスを完全終了
    }, 500);
});


app.delete('/unregister', (req, res) => {
  const { address } = req.body;
  servers = servers.filter(s => s.address !== address);
  res.send({ success: true });
});

const TIMEOUT = 20 * 1000; // 20秒応答なしで削除

setInterval(() => {
  const now = Date.now();
  console.log("---- PRUNE CHECK ----", new Date().toISOString());
  const before = servers.length;
servers.forEach(s => {
    console.log(
      s.name,
      "elapsed(s):",
      ((now - s.time) / 1000).toFixed(1)
    );
  });
  servers = servers.filter(s => now - s.time < TIMEOUT);

  const after = servers.length;
  if (before !== after) {
    console.log(`Removed ${before - after} dead server(s)`);
  }
}, 10 * 1000); // 10秒ごとにチェック

app.listen(3000, "0.0.0.0", () => {
  console.log('Server is running on http://0.0.0.0:3000');});