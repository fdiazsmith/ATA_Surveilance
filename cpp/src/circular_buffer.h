// circular_buffer.h

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <vector>
#include <mutex>
#include <gst/gst.h>

class CircularBuffer {
public:
    CircularBuffer(size_t size);

    void addFrame(GstBuffer* frame);
    GstBuffer* getFrame(size_t delay_frames);

private:
    std::vector<GstBuffer*> buffer;
    size_t head;
    size_t tail;
    bool full;
    std::mutex mutex;
};

#endif // CIRCULAR_BUFFER_H
