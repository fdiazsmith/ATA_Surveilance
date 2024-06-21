#!/bin/bash

# Set the PKG_CONFIG_PATH if necessary
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH

# Verify if opencv4.pc exists and add its path to PKG_CONFIG_PATH if necessary
if [ ! -f /usr/lib/x86_64-linux-gnu/pkgconfig/opencv4.pc ]; then
    echo "opencv4.pc not found in /usr/lib/x86_64-linux-gnu/pkgconfig, please ensure OpenCV is installed correctly."
    exit 1
fi

# Set the source files
SOURCE_FILES="src/main.cpp src/opengl_utils.cpp src/gstreamer_utils.cpp src/circular_buffer.cpp"

# Set the output executable name
OUTPUT_FILE="build/ATA_Surveillance"

# Set the library paths
LIBRARY_PATHS="/usr/local/lib /usr/lib/x86_64-linux-gnu"

# Set the include paths
INCLUDE_PATHS="/usr/include /usr/include/gstreamer-1.0 /usr/include/glib-2.0 /usr/lib/x86_64-linux-gnu/glib-2.0/include /usr/include/opencv4"

# Set the libraries to link
LIBRARIES="-lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 -lgstbase-1.0 -lgstvideo-1.0 -lgstapp-1.0 -lGLEW -lGLFW -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lGL -lGLU -lpthread"

# Set additional OpenCV libraries
OPENCV_LIBRARIES="-lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_objdetect -lopencv_ml -lopencv_photo"

# Build the command to compile with static linking
COMPILE_CMD="g++ -o $OUTPUT_FILE $SOURCE_FILES -static"

# Add include paths
for INCLUDE_PATH in $INCLUDE_PATHS; do
    COMPILE_CMD="$COMPILE_CMD -I$INCLUDE_PATH"
done

# Add library paths
for LIB_PATH in $LIBRARY_PATHS; do
    COMPILE_CMD="$COMPILE_CMD -L$LIB_PATH"
done

# Add libraries
COMPILE_CMD="$COMPILE_CMD $LIBRARIES $OPENCV_LIBRARIES"

# Add pkg-config flags
COMPILE_CMD="$COMPILE_CMD $(pkg-config --cflags --libs glew glfw3 gstreamer-1.0 gstreamer-app-1.0 opencv4)"

# Print and execute the compile command
echo "Compiling with the following command:"
echo $COMPILE_CMD

$COMPILE_CMD

# Check if the compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Output file: $OUTPUT_FILE"
else
    echo "Compilation failed."
fi
