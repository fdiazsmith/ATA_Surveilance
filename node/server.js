const ffmpeg = require('fluent-ffmpeg');
const WebSocket = require('ws');

ffmpeg.setFfmpegPath('C:/Users/fdiaz/ffmpeg/bin/ffmpeg.exe');

const cameraStreamUrl = 'rtsp://admin:anna.landa85@10.0.0.26:554/h264Preview_01_main';
const wsServer = new WebSocket.Server({ port: 8082 });

wsServer.on('connection', (ws) => {
    const ffmpegProcess = ffmpeg(cameraStreamUrl)
        .outputFormat('mpegts')
        .videoCodec('libx264')
        .noAudio()
        .on('end', () => {
            console.log('Stream ended');
        })
        .on('error', (err) => {
            console.error('An error occurred:', err.message);
        });

    ffmpegProcess.pipe(ws);
});

console.log('WebSocket server is running on ws://localhost:8082');
