#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
# delete the executable if it exists
rm build/gstreamer_opengl

# Set the PKG_CONFIG_PATH if necessary
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

# Compile the project
g++ -o build/gstreamer_opengl src/main.cpp src/opengl_utils.cpp src/gstreamer_utils.cpp src/circular_buffer.cpp `pkg-config --cflags --libs glew glfw3 gstreamer-1.0 gstreamer-app-1.0 opencv` -lGLESv2

echo "Compilation finished. Executable is located in the build directory."

# Run the executable
./build/gstreamer_opengl
