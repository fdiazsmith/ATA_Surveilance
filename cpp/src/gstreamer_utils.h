// gstreamer_utils.h
#ifndef GSTREAMER_UTILS_H
#define GSTREAMER_UTILS_H

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <GLFW/glfw3.h>
#include <GLES2/gl2.h>
#include <opencv2/opencv.hpp>
#include "circular_buffer.h"


struct FrameBuffer {
    GstSample* sample;
    bool new_frame;
    std::mutex mtx;
    std::condition_variable cv;
};

struct AppData {
    GLFWwindow* window;
    GLuint program;
    GLuint tex_rtsp;
    GLuint tex_local;
    GLuint vbo, ebo;
    GstElement* pipeline_rtsp;
    GstElement* pipeline_local;
    GstElement* appsink_rtsp;
    GstElement* appsink_local;
    FrameBuffer rtsp_buffer;
    FrameBuffer local_buffer;
    bool transitioning;
    float alpha;
    float target_alpha;
    int frame_count;
    std::chrono::steady_clock::time_point transition_start;
    std::chrono::steady_clock::time_point last_time;
    int texture_width;  
    int texture_height; 
    CircularBuffer circularBuffer;
    AppData() : circularBuffer(20) {} // Initialize with desired buffer size
};

void init_gstreamer(AppData* app, const std::string& rtsp_url, int width, int height, bool delay_video, int video_delay);

void load_texture_from_buffer(AppData* app, size_t delay_frames);
void load_texture_from_mat(const cv::Mat& img, GLuint textureID);


void render(AppData* app);
void update(AppData* app);

#endif // GSTREAMER_UTILS_H
