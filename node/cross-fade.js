const onvif = require('onvif');
const { Cam } = onvif;
const { spawn } = require('child_process');

// Camera details
const cameraIp = '10.0.0.26';
const cameraPort = 8000;
const username = 'admin';
const password = 'anna.landa85';
const streamUri = 'rtsp://10.0.0.26:554/Preview_01_main'; // Update this to your camera's RTSP stream URL

// Initialize ONVIF camera
const camera = new Cam({
    hostname: cameraIp,
    port: cameraPort,
    username: username,
    password: password
}, function (err) {
    if (err) {
        console.error('Connection Failed:', err);
        return;
    }
    console.log('Connected to ONVIF camera.');

    // Start the FFmpeg process to display the stream in full screen
    const ffmpeg = spawn('C:/Users/fdiaz/ffmpeg/bin/ffmpeg', [
        '-i', streamUri,          // Input stream URL
        '-vf', 'scale=1920:1080', // Scale to full HD
        '-f', 'sdl',              // Use SDL output
        'camera_stream'           // Output window name
    ]);

    ffmpeg.stdout.on('data', (data) => {
        console.log(`stdout: ${data}`);
    });

    ffmpeg.stderr.on('data', (data) => {
        console.error(`stderr: ${data}`);
    });

    ffmpeg.on('close', (code) => {
        console.log(`FFmpeg process exited with code ${code}`);
    });
});
