// g++ -o gstreamer_opengl_basic gstreamer_opengl_basic.cpp `pkg-config --cflags --libs glfw3 gstreamer-1.0 gstreamer-app-1.0` -lGLESv2

#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <iostream>

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
    uniform sampler2D texture;
    void main() {
        gl_FragColor = texture2D(texture, v_texcoord);
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

struct AppData {
    GLFWwindow* window;
    GLuint program;
    GLuint texture;
    GLuint vbo, ebo;
    GstElement* pipeline;
    GstElement* appsink;
    bool new_frame;
    GstSample* sample;
};

static GstFlowReturn new_frame_callback(GstAppSink* sink, gpointer data) {
    AppData* app = static_cast<AppData*>(data);
    app->new_frame = true;
    app->sample = gst_app_sink_pull_sample(sink);
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

    app->pipeline = gst_parse_launch("rtspsrc location=rtsp://admin:anna.landa85@10.0.0.26:554/Preview_01_sub ! decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB,width=800,height=600 ! appsink name=appsink", NULL);
    app->appsink = gst_bin_get_by_name(GST_BIN(app->pipeline), "appsink");

    GstAppSinkCallbacks callbacks = { NULL, NULL, new_frame_callback };
    gst_app_sink_set_callbacks(GST_APP_SINK(app->appsink), &callbacks, app, NULL);

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(app->pipeline));
    gst_bus_add_watch(bus, bus_call, g_main_loop_new(NULL, FALSE));
    gst_object_unref(bus);

    gst_element_set_state(app->pipeline, GST_STATE_PLAYING);
}

void render(AppData* app) {
    if (app->new_frame) {
        GstBuffer* buffer = gst_sample_get_buffer(app->sample);
        GstMapInfo map;
        gst_buffer_map(buffer, &map, GST_MAP_READ);

        update_texture(app->texture, map.data, 800, 600);

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(app->sample);

        app->new_frame = false;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->program);
    glBindBuffer(GL_ARRAY_BUFFER, app->vbo);
    glBindTexture(GL_TEXTURE_2D, app->texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glfwSwapBuffers(app->window);
}

int main() {
    AppData app = {};
    app.new_frame = false;

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
    app.texture = create_texture(800, 600);

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
        render(&app);
    }

    glfwDestroyWindow(app.window);
    glfwTerminate();
    gst_element_set_state(app.pipeline, GST_STATE_NULL);
    gst_object_unref(app.pipeline);

    return 0;
}
