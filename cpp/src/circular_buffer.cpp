// circular_buffer.cpp

#include "circular_buffer.h"

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
}

GstBuffer* CircularBuffer::getFrame(size_t delay_frames) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!full && head <= delay_frames) {
        return nullptr; // Not enough frames in buffer
    }
    size_t index = (tail + delay_frames) % buffer.size();
    return gst_buffer_ref(buffer[index]);
}
