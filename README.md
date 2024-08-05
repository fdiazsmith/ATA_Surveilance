# ATA_Surveilance

## how to start the program 
1. simplest way
* power cycle the pi.
2. manually
* open terminal
* paste the following command and hit enter
```bash
cd /home/pi/Documents/ATA_Surveilance && ./start.sh
```

## how to edit the configuration
1. quit the app if it is running, by pressing `q` 
2. open the configuration file with text editor by double clicking on it. the file is in `/Documents/ATA_Surveilance/config.txt`
3. save the file and close the text editor
4. run either of the above methods to start the app

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.

.




how it autoruns on boot

#### Executing on start

1. Edit the systemd Service File
Open the service file for editing:

bash
Copy code
```
sudo nano /etc/systemd/system/ata_surveillance.service
```
Modify the Service Definition:

Update the ExecStart line to open the terminal and run the script. Here's an example using lxterminal:


```
[Unit]
Description=ATA Surveillance Service
After=graphical.target

[Service]
ExecStart=/usr/bin/lxterminal -e /bin/bash -c '/home/pi/Documents/ATA_Surveilance/start.sh; exec bash'
WorkingDirectory=/home/pi/Documents/ATA_Surveilance
StandardOutput=inherit
StandardError=inherit
Restart=always
User=pi
Environment=DISPLAY=:0

[Install]
WantedBy=graphical.target
```
ExecStart: Launches lxterminal and runs the script. The `-e` option specifies the command to execute inside the terminal. The command `/bin/bash -c '/home/pi/Documents/ATA_Surveilance/start.sh; exec bash'` ensures that the terminal stays open after the script completes.
`Environment=DISPLAY=:0` Sets the DISPLAY environment variable, assuming the default display is :0. Adjust this if your setup uses a different display.
Save and Exit:

Save the changes and exit the text editor (Ctrl+X, then Y, then Enter).

3. Reload systemd and Restart the Service
Reload systemd to apply the changes:

bash
Copy code
```
sudo systemctl daemon-reload
```
Start the service:

bash
Copy code
```
sudo systemctl start ata_surveillance.service
```
