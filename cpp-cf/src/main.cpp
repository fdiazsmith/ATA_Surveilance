#include "opengl_utils.h"
#include "gstreamer_utils.h"
#include "circular_buffer.h"
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

void load_config(std::string& rtsp_url,  int& width, int& height, bool& fullscreen, bool& delay_video, int& video_delay) {
    std::ifstream config_file("/home/pi/Documents/ATA_Surveilance/config.txt");
    std::string line;
    while (std::getline(config_file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "RTSP_URL") {
                rtsp_url = value;
            } else if (key == "LOCAL_VIDEO_PATH") {
                // local_video_path = value; // Comment out this line
            } else if (key == "VIDEO_WIDTH") {
                width = std::stoi(value);
            } else if (key == "VIDEO_HEIGHT") {
                height = std::stoi(value);
            } else if (key == "FULLSCREEN") {
                fullscreen = (value == "true" || value == "1");
            } else if (key == "DELAY_VIDEO") {
                delay_video = (value == "true" || value == "1");
            } else if (key == "VIDEO_DELAY") {
                video_delay = std::stoi(value);
            }
        }
    }
    // print out our config variables to the console
    std::cout << "RTSP_URL: " << rtsp_url << std::endl;
    std::cout << "VIDEO_WIDTH: " << width << std::endl;
    std::cout << "VIDEO_HEIGHT: " << height << std::endl;
    std::cout << "FULLSCREEN: " << fullscreen << std::endl;
    std::cout << "DELAY_VIDEO: " << delay_video << std::endl;
    std::cout << "VIDEO_DELAY: " << video_delay << std::endl;
    
}




int main() {
    std::string rtsp_url;
    // std::string local_video_path; // Comment out this line
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    bool delay_video = false;
    int video_delay = 0;

    load_config(rtsp_url, width, height, fullscreen, delay_video, video_delay);

    AppData app(video_delay * 20);
    app.alpha = 0.0f;
    app.target_alpha = 0.0f;
    app.transitioning = false;
    app.frame_count = 0;
    app.last_time = std::chrono::steady_clock::now();
    app.texture_width = width;  // Initialize texture dimensions
    app.texture_height = height; // Initialize texture dimensions
    // app.circularBuffer = CircularBuffer(video_delay*30); // Initialize with 600 frames (assuming 10 seconds delay at 30 fps)
  

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : NULL;
    const GLFWvidmode* mode = fullscreen ? glfwGetVideoMode(monitor) : NULL;

    if (fullscreen && mode) {
        width = mode->width;
        height = mode->height;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

    app.window = glfwCreateWindow(width, height, "OpenGL ES GStreamer Test", monitor, NULL); // Full-screen mode
    
    if (!app.window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(app.window);
    glfwSwapInterval(1);

    app.program = create_shader_program("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    app.tex_rtsp = create_texture(app.texture_width, app.texture_height);
    // app.tex_local = create_texture(app.texture_width, app.texture_height); // Comment out this line

    GLfloat vertices[] = {
        // Positions       // Texture Coords
        -1.0f,  1.0f,      0.0f, 1.0f,
        -1.0f, -1.0f,      0.0f, 0.0f,
         1.0f, -1.0f,      1.0f, 0.0f,
         1.0f,  1.0f,      1.0f, 1.0f
    };
    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenBuffers(1, &app.vbo);
    glGenBuffers(1, &app.ebo);

    glBindBuffer(GL_ARRAY_BUFFER, app.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint posAttrib = glGetAttribLocation(app.program, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

    GLint texAttrib = glGetAttribLocation(app.program, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    init_gstreamer(&app, rtsp_url, width, height, delay_video, video_delay);

    auto start_time = std::chrono::steady_clock::now(); // Start time for the application

    while (!glfwWindowShouldClose(app.window)) {
        glfwPollEvents();

        if (glfwGetKey(app.window, GLFW_KEY_A) == GLFW_PRESS && !app.transitioning) {
            app.target_alpha = 1.0f;
            app.transition_start = std::chrono::steady_clock::now();
            app.transitioning = true;
        }
        if (glfwGetKey(app.window, GLFW_KEY_D) == GLFW_PRESS && !app.transitioning) {
            app.target_alpha = 0.0f;
            app.transition_start = std::chrono::steady_clock::now();
            app.transitioning = true;
        }

        // Check if 'q' key is pressed and set the window to close
        if (glfwGetKey(app.window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(app.window, GLFW_TRUE);
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        float time_seconds = static_cast<float>(elapsed) / 1000.0f;
        set_uniform_float(app.program, "time", time_seconds);

        update(&app);
    }

    glfwDestroyWindow(app.window);
    glfwTerminate();
    gst_element_set_state(app.pipeline_rtsp, GST_STATE_NULL);
    // gst_element_set_state(app.pipeline_local, GST_STATE_NULL); // Comment out this line
    gst_object_unref(app.pipeline_rtsp);
    // gst_object_unref(app.pipeline_local); // Comment out this line

    return 0;
}