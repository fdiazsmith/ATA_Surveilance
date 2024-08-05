#!/bin/bash

# Load configuration
source config.txt

echo "Starting video player..."
delay 15


# Example commands using the configuration variables
echo "RTSP URL: $RTSP_URL"
echo "Video Width: $VIDEO_WIDTH"
echo "Video Height: $VIDEO_HEIGHT"
echo "Fullscreen: $FULLSCREEN"
echo "Delay Video: $DELAY_VIDEO"
echo "Video Delay: $VIDEO_DELAY"

# delay  the execution of the script by 10 seconds
sleep 5

# Example command to play a video
if [ "$DELAY_VIDEO" = true ]; then
  echo "Playing video feed with delay of  $VIDEO_DELAY seconds..."
  sleep 5
  cd ./cpp-cf
  bash ./compile.sh
else
  echo "Playing video feed without delay."
  sleep 5
  cd ./cpp
  bash ./compile.sh
fi

# Example command to open video (replace this with your actual command)
if [ "$FULLSCREEN" = true ]; then
  echo "Playing video in fullscreen mode."
  # Replace this with your actual video player command
  # Example: omxplayer --win "0 0 $VIDEO_WIDTH $VIDEO_HEIGHT" --avdict rtsp_transport:tcp "$RTSP_URL"
else
  echo "Playing video in windowed mode with resolution ${VIDEO_WIDTH}x${VIDEO_HEIGHT}."
  # Replace this with your actual video player command
  # Example: omxplayer --win "0 0 $VIDEO_WIDTH $VIDEO_HEIGHT" --avdict rtsp_transport:tcp "$RTSP_URL"
fi
