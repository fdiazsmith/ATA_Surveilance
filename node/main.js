const onvif = require('onvif');
const { Cam } = onvif;

// Camera details
const cameraIp = '10.0.0.26';
const cameraPort = 8000;
const username = 'admin';
const password = 'anna.landa85';

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
        // console.log('Capabilities:', capabilities);
        // cb = function(err, result) {};
        // camera.createPullPointSubscription({}, cb );
        camera.on('event', function (result) {
            console.log('Event:', result, '\n');
            m = result.message.message;
            for (let key in m) {
                console.log(key, m[key]);
            }

        });
        // Create PullPointSubscription
        // camera.createPullPointSubscription({}, function(err, subscription)  {
        //     if (err) {
        //         console.error('Error creating PullPointSubscription:', err);
        //         return;
        //     }
        //     console.log('PullPointSubscription created:', subscription);

        //     // Pull Messages
        //     setInterval(() => {
        //         camera.pullMessages({
        //             timeout: 'PT5S',
        //             messageLimit: 10
        //         }, (err, messages) => {
        //             if (err) {
        //                 console.error('Error pulling messages:', err);
        //                 return;
        //             }
        //             console.log('Pulled messages:', messages);
        //         });
        //     }, 5000); // Poll every 5 seconds
        // });
    });
});
