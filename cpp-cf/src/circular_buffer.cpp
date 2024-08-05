#include "circular_buffer.h"
#include <iostream>

CircularBuffer::CircularBuffer(size_t size) : buffer(size), head(0), tail(0), full(false) {}

void CircularBuffer::addFrame(GstBuffer* frame) {
    std::lock_guard<std::mutex> lock(mutex);

    if (buffer[head] != nullptr) {
        gst_buffer_unref(buffer[head]);
    }

    buffer[head] = gst_buffer_ref(frame);
    head = (head + 1) % buffer.size();

    if (full) {
        tail = (tail + 1) % buffer.size();
    }

    full = head == tail;

    // std::cout << "Frame added to circular buffer at position: " << head << std::endl; // Debug statement
}

GstBuffer* CircularBuffer::getFrame(size_t delay_frames) {
    std::lock_guard<std::mutex> lock(mutex);

    if (!full && head <= delay_frames) {
        std::cout << "Not enough frames in buffer to retrieve" << std::endl; // Debug statement
        return nullptr; // Not enough frames in buffer
    }

    size_t index = (tail + delay_frames) % buffer.size();
    // std::cout << "Retrieving frame from circular buffer at position: " << index << std::endl; // Debug statement

    return gst_buffer_ref(buffer[index]);
}
