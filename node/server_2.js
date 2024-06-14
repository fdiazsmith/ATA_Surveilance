const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const RTSPServer = require('rtsp-streaming-server').default;

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// app.use(express.static('public'));
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index_2.html');
    console.log('index_2.html');
});

const rtspServer = new RTSPServer({
    serverPort: 8554,
    clientPort: 8080,
    streamUrl: 'rtsp://admin:anna.landa85@10.0.0.26:554/h264Preview_01_main'
});

rtspServer.start().then(() => {
    console.log('RTSP server is running...');
}).catch(err => {
    console.error('Failed to start RTSP server:', err);
});

wss.on('connection', (ws) => {
    console.log('Client connected');

    ws.on('message', (message) => {
        console.log('Received message:', message);
    });

    rtspServer.on('data', (data) => {
        ws.send(data);
    });

    rtspServer.on('error', (error) => {
        console.error('Stream error:', error);
    });
});

server.listen(8084, () => {
    console.log('Server is listening on port 8084');
});