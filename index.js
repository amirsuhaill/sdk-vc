const WebSocket = require('ws');

let data = process.argv.slice(2);
let i = 0;
const time = 5000

if (data.length === 0) {
    console.error("No data provided");
    process.exit(1);
}

console.log("Data:", data);

const wss = new WebSocket.Server({ port: 3000 });

wss.on('connection', (ws) => {
    console.log("Client connected");

    const interval = setInterval(() => {
        const msg = data[i % data.length];
        ws.send(msg);
        console.log("Sent:", msg);
        i++;
    }, time);

    ws.on('close', () => {
        console.log("Client disconnected");
        clearInterval(interval);
    });

    ws.on('error', (err) => {
        console.error("WebSocket error:", err);
    });
});

console.log("WebSocket server running on port 3000");


// node index.js my name is amir suhail
