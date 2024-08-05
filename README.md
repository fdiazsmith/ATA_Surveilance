# ATA_Surveilance
 
#### Executing on start

1. Using rc.local File
Edit rc.local:
The rc.local file is a traditional method for running commands at startup. You can add your script to this file to run it on boot.

bash
```
sudo nano /etc/rc.local```
Add Your Script:
Add the following line before the exit 0 line in the file. Make sure to provide the full path to your script.

bash

```
/home/pi/Documents/ATA_Surveilance/start.sh &
```
The & at the end runs the script in the background.

Save and Exit:
Press Ctrl+X, then Y, and then Enter to save and exit.