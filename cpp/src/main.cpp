#include "opengl_utils.h"
#include "gstreamer_utils.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void load_config(std::string& rtsp_url, std::string& local_video_path, int& width, int& height) {
    std::ifstream config_file("src/config.txt");
    std::string line;
    while (std::getline(config_file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "RTSP_URL") {
                rtsp_url = value;
            } else if (key == "LOCAL_VIDEO_PATH") {
                local_video_path = value;
            } else if (key == "VIDEO_WIDTH") {
                width = std::stoi(value);
            } else if (key == "VIDEO_HEIGHT") {
                height = std::stoi(value);
            }
        }
    }
}

int main() {
    std::string rtsp_url;
    std::string local_video_path;
    int width = 800;
    int height = 600;

    load_config(rtsp_url, local_video_path, width, height);

    AppData app = {};
    app.alpha = 0.0f;
    app.target_alpha = 0.0f;
    app.transitioning = false;
    app.frame_count = 0;
    app.last_time = std::chrono::steady_clock::now();

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

    app.window = glfwCreateWindow(width, height, "OpenGL ES GStreamer Test", NULL, NULL);
    if (!app.window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(app.window);
    glfwSwapInterval(1);

    app.program = create_shader_program("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    app.tex_rtsp = create_texture(width, height);
    app.tex_local = create_texture(width, height);

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

    init_gstreamer(&app, rtsp_url, local_video_path, width, height);

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

        update(&app);
    }

    glfwDestroyWindow(app.window);
    glfwTerminate();
    gst_element_set_state(app.pipeline_rtsp, GST_STATE_NULL);
    gst_element_set_state(app.pipeline_local, GST_STATE_NULL);
    gst_object_unref(app.pipeline_rtsp);
    gst_object_unref(app.pipeline_local);

    return 0;
}
