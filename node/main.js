const onvif = require('onvif');
const { Cam } = onvif;
const http = require('http');
const socketIo = require('socket.io');
const robot = require('robotjs');  // Import robotjs

// Camera details
const cameraIp = '10.0.0.26';
const cameraPort = 8000;
const username = 'admin';
const password = 'anna.landa85';

// Create an HTTP server
const server = http.createServer();
const io = socketIo(server);

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

    // Get camera capabilities
    camera.getCapabilities((err, capabilities) => {
        if (err) {
            console.error('Error getting capabilities:', err);
            return;
        }
        
        camera.on('event', function (result) {
            console.log('Event:', result, '\n');
            let m = result.message.message;

            // Emit the message to all connected clients
            io.emit('cameraEvent', m);
            
            // Emulate a keyboard event, press 'd' key
            robot.keyTap('d');

            // Wait 5 seconds and emit another keyboard event, press 'a' key
            setTimeout(() => {
                robot.keyTap('a');
            }, 5000);
        });
    });
});

// Listen for client connections
io.on('connection', (socket) => {
    console.log('Client connected');
    socket.on('disconnect', () => {
        console.log('Client disconnected');
    });
});

// Start the HTTP server
const PORT = 3000;
server.listen(PORT, () => {
    console.log(`Server is listening on port ${PORT}`);
});
