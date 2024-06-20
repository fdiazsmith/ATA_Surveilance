#!/bin/bash

# Update package list
sudo apt-get update

# Install necessary libraries
sudo apt-get install -y \
    libarmmem-v7l \
    libglew2.1 \
    libglu1-mesa \
    libgl1-mesa-glx \
    libglfw3 \
    libgstreamer1.0-0 \
    libgstreamer-plugins-base1.0-0 \
    libgobject-2.0-0 \
    libglib2.0-0 \
    libgles2-mesa \
    libstdc++6 \
    libm6 \
    libgcc1 \
    libpthread0 \
    libc6 \
    libx11-6 \
    libgl1 \
    libglx0 \
    librt1 \
    libdl2 \
    libxrandr2 \
    libxinerama1 \
    libxxf86vm1 \
    libxcursor1 \
    libgmodule-2.0-0 \
    libffi6 \
    libpcre3 \
    libxcb1 \
    libxext6 \
    libxrender1 \
    libxfixes3 \
    libxau6 \
    libxdmcp6 \
    libbsd0

# Clean up
sudo apt-get clean

echo "All necessary libraries have been installed."
