const express = require('express');
const app = express();

let data = [];

process.argv.slice(2).forEach(val => {
    data.push(val);
});

if (data.length === 0) {
    console.error("No data provided");
    process.exit(1);
}

console.log("Data:", data);

let i = 0;

app.get('/data', (req, res) => {
    console.log("Request received");
    res.send(data[i % data.length]);
    i++;
});

app.listen(3000, '0.0.0.0', () => {
    console.log('Server running on port 3000');
});

// node index.js my name is amir suhail
