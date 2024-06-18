#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <FreeImage.h>

const char* vertex_shader_source = R"(
    attribute vec4 position;
    attribute vec2 texcoord;
    varying vec2 v_texcoord;
    void main() {
        gl_Position = position;
        v_texcoord = texcoord;
    }
)";

const char* fragment_shader_source = R"(
    precision mediump float;
    varying vec2 v_texcoord;
    uniform sampler2D tex1;
    uniform sampler2D tex2;
    uniform float alpha;
    void main() {
        vec4 color1 = texture2D(tex1, v_texcoord);
        vec4 color2 = texture2D(tex2, v_texcoord);
        gl_FragColor = mix(color1, color2, alpha);
    }
)";

GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << info_log << std::endl;
    }
    return shader;
}

GLuint create_shader_program() {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

GLuint create_texture(GLsizei width, GLsizei height) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void update_texture(GLuint texture, const void* data, GLsizei width, GLsizei height) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

struct FrameBuffer {
    std::mutex mtx;
    std::condition_variable cv;
    GstSample* sample[2] = {nullptr, nullptr};
    bool new_frame = false;
    int current = 0;
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
    float alpha;
    float target_alpha;
    std::chrono::steady_clock::time_point transition_start;
    bool transitioning;
    int frame_count;
    std::chrono::steady_clock::time_point last_time;
};

static GstFlowReturn new_frame_callback_rtsp(GstAppSink* sink, gpointer data) {
    AppData* app = static_cast<AppData*>(data);
    {
        std::lock_guard<std::mutex> lock(app->rtsp_buffer.mtx);
        int next = (app->rtsp_buffer.current + 1) % 2;
        if (app->rtsp_buffer.sample[next]) {
            gst_sample_unref(app->rtsp_buffer.sample[next]);
        }
        app->rtsp_buffer.sample[next] = gst_app_sink_pull_sample(sink);
        app->rtsp_buffer.new_frame = true;
        app->rtsp_buffer.current = next;
    }
    app->rtsp_buffer.cv.notify_one();
    return GST_FLOW_OK;
}

static GstFlowReturn new_frame_callback_local(GstAppSink* sink, gpointer data) {
    AppData* app = static_cast<AppData*>(data);
    {
        std::lock_guard<std::mutex> lock(app->local_buffer.mtx);
        int next = (app->local_buffer.current + 1) % 2;
        if (app->local_buffer.sample[next]) {
            gst_sample_unref(app->local_buffer.sample[next]);
        }
        app->local_buffer.sample[next] = gst_app_sink_pull_sample(sink);
        app->local_buffer.new_frame = true;
        app->local_buffer.current = next;
    }
    app->local_buffer.cv.notify_one();
    return GST_FLOW_OK;
}

static gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data) {
    GMainLoop* loop = static_cast<GMainLoop*>(data);
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError* err;
            gchar* debug;
            gst_message_parse_error(msg, &err, &debug);
            g_printerr("Error: %s\n", err->message);
            g_error_free(err);
            g_free(debug);
            g_main_loop_quit(loop);
            break;
        }
        case GST_MESSAGE_EOS:
            g_main_loop_quit(loop);
            break;
        default:
            break;
    }
    return TRUE;
}

void init_gstreamer(AppData* app) {
    gst_init(NULL, NULL);

    app->pipeline_rtsp = gst_parse_launch("rtspsrc location=rtsp://admin:anna.landa85@10.0.0.26:554/Preview_01_main ! decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB,width=800,height=600 ! appsink name=appsink_rtsp", NULL);
    app->appsink_rtsp = gst_bin_get_by_name(GST_BIN(app->pipeline_rtsp), "appsink_rtsp");

    app->pipeline_local = gst_parse_launch("filesrc location=../../assets/static.mp4 ! decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB,width=800,height=600 ! appsink name=appsink_local", NULL);
    app->appsink_local = gst_bin_get_by_name(GST_BIN(app->pipeline_local), "appsink_local");

    GstAppSinkCallbacks callbacks_rtsp = { NULL, NULL, new_frame_callback_rtsp };
    gst_app_sink_set_callbacks(GST_APP_SINK(app->appsink_rtsp), &callbacks_rtsp, app, NULL);

    GstAppSinkCallbacks callbacks_local = { NULL, NULL, new_frame_callback_local };
    gst_app_sink_set_callbacks(GST_APP_SINK(app->appsink_local), &callbacks_local, app, NULL);

    GstBus* bus_rtsp = gst_pipeline_get_bus(GST_PIPELINE(app->pipeline_rtsp));
    gst_bus_add_watch(bus_rtsp, bus_call, g_main_loop_new(NULL, FALSE));
    gst_object_unref(bus_rtsp);

    GstBus* bus_local = gst_pipeline_get_bus(GST_PIPELINE(app->pipeline_local));
    gst_bus_add_watch(bus_local, bus_call, g_main_loop_new(NULL, FALSE));
    gst_object_unref(bus_local);

    gst_element_set_state(app->pipeline_rtsp, GST_STATE_PLAYING);
    gst_element_set_state(app->pipeline_local, GST_STATE_PLAYING);
}

void render(AppData* app) {
    bool rtsp_frame_updated = false;
    bool local_frame_updated = false;

    {
        std::unique_lock<std::mutex> lock(app->rtsp_buffer.mtx);
        if (app->rtsp_buffer.new_frame) {
            GstBuffer* buffer = gst_sample_get_buffer(app->rtsp_buffer.sample[app->rtsp_buffer.current]);
            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_READ);

            update_texture(app->tex_rtsp, map.data, 800, 600);

            gst_buffer_unmap(buffer, &map);
            gst_sample_unref(app->rtsp_buffer.sample[app->rtsp_buffer.current]);
            app->rtsp_buffer.sample[app->rtsp_buffer.current] = nullptr;

            app->rtsp_buffer.new_frame = false;
            rtsp_frame_updated = true;
        }
    }

    {
        std::unique_lock<std::mutex> lock(app->local_buffer.mtx);
        if (app->local_buffer.new_frame) {
            GstBuffer* buffer = gst_sample_get_buffer(app->local_buffer.sample[app->local_buffer.current]);
            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_READ);

            update_texture(app->tex_local, map.data, 800, 600);

            gst_buffer_unmap(buffer, &map);
            gst_sample_unref(app->local_buffer.sample[app->local_buffer.current]);
            app->local_buffer.sample[app->local_buffer.current] = nullptr;

            app->local_buffer.new_frame = false;
            local_frame_updated = true;
        }
    }

    if (app->transitioning) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - app->transition_start).count();
        float progress = static_cast<float>(elapsed) / 1000.0f; // 1 second transition
        if (progress >= 1.0f) {
            progress = 1.0f;
            app->transitioning = false;
        }
        app->alpha = (app->target_alpha == 1.0f) ? progress : 1.0f - progress;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->program);
    glBindBuffer(GL_ARRAY_BUFFER, app->vbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->tex_rtsp);
    glUniform1i(glGetUniformLocation(app->program, "tex1"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, app->tex_local);
    glUniform1i(glGetUniformLocation(app->program, "tex2"), 1);

    glUniform1f(glGetUniformLocation(app->program, "alpha"), app->alpha);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glfwSwapBuffers(app->window);

    std::cout << "Render completed: RTSP frame updated = " << rtsp_frame_updated << ", Local frame updated = " << local_frame_updated << std::endl;
}

void update(AppData* app) {
    render(app);
    std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Approximately 60 FPS

    app->frame_count++;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - app->last_time).count();
    if (elapsed >= 1000) {
        double fps = static_cast<double>(app->frame_count) / (elapsed / 1000.0);
        std::cout << "FPS: " << fps << std::endl;
        app->frame_count = 0;
        app->last_time = now;
    }
}

int main() {
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

    app.window = glfwCreateWindow(800, 600, "OpenGL ES GStreamer Test", NULL, NULL);
    if (!app.window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(app.window);
    glfwSwapInterval(1);

    app.program = create_shader_program();
    app.tex_rtsp = create_texture(800, 600);
    app.tex_local = create_texture(800, 600);

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

    init_gstreamer(&app);

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
