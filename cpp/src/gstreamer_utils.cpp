#include "gstreamer_utils.h"
#include "opengl_utils.h"
#include <iostream>
#include <chrono>
#include <thread>

static GstFlowReturn new_frame_callback_rtsp(GstAppSink* sink, gpointer data) {
    AppData* app = static_cast<AppData*>(data);
    {
        std::lock_guard<std::mutex> lock(app->rtsp_buffer.mtx);
        if (app->rtsp_buffer.sample) {
            gst_sample_unref(app->rtsp_buffer.sample);
        }
        app->rtsp_buffer.sample = gst_app_sink_pull_sample(sink);
        app->rtsp_buffer.new_frame = true;
    }
    app->rtsp_buffer.cv.notify_one();
    return GST_FLOW_OK;
}

static GstFlowReturn new_frame_callback_local(GstAppSink* sink, gpointer data) {
    AppData* app = static_cast<AppData*>(data);
    {
        std::lock_guard<std::mutex> lock(app->local_buffer.mtx);
        if (app->local_buffer.sample) {
            gst_sample_unref(app->local_buffer.sample);
        }
        app->local_buffer.sample = gst_app_sink_pull_sample(sink);
        app->local_buffer.new_frame = true;
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

void init_gstreamer(AppData* app, const std::string& rtsp_url, const std::string& local_video_path, int width, int height) {
    gst_init(NULL, NULL);

    std::string rtsp_pipeline = "rtspsrc location=" + rtsp_url + " ! decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB,width=" + std::to_string(width) + ",height=" + std::to_string(height) + " ! appsink name=appsink_rtsp";
    app->pipeline_rtsp = gst_parse_launch(rtsp_pipeline.c_str(), NULL);
    app->appsink_rtsp = gst_bin_get_by_name(GST_BIN(app->pipeline_rtsp), "appsink_rtsp");

    // std::string local_pipeline = "filesrc location=" + local_video_path + " ! decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB,width=" + std::to_string(width) + ",height=" + std::to_string(height) + " ! appsink name=appsink_local"; // Comment out this line
    // app->pipeline_local = gst_parse_launch(local_pipeline.c_str(), NULL); // Comment out this line
    // app->appsink_local = gst_bin_get_by_name(GST_BIN(app->pipeline_local), "appsink_local"); // Comment out this line

    GstAppSinkCallbacks callbacks_rtsp = { NULL, NULL, new_frame_callback_rtsp };
    gst_app_sink_set_callbacks(GST_APP_SINK(app->appsink_rtsp), &callbacks_rtsp, app, NULL);

    // GstAppSinkCallbacks callbacks_local = { NULL, NULL, new_frame_callback_local }; // Comment out this line
    // gst_app_sink_set_callbacks(GST_APP_SINK(app->appsink_local), &callbacks_local, app, NULL); // Comment out this line

    GstBus* bus_rtsp = gst_pipeline_get_bus(GST_PIPELINE(app->pipeline_rtsp));
    gst_bus_add_watch(bus_rtsp, bus_call, g_main_loop_new(NULL, FALSE));
    gst_object_unref(bus_rtsp);

    // GstBus* bus_local = gst_pipeline_get_bus(GST_PIPELINE(app->pipeline_local)); // Comment out this line
    // gst_bus_add_watch(bus_local, bus_call, g_main_loop_new(NULL, FALSE)); // Comment out this line
    // gst_object_unref(bus_local); // Comment out this line

    gst_element_set_state(app->pipeline_rtsp, GST_STATE_PLAYING);
    // gst_element_set_state(app->pipeline_local, GST_STATE_PLAYING); // Comment out this line
}


void render(AppData* app) {
    {
        std::unique_lock<std::mutex> lock(app->rtsp_buffer.mtx);
        if (app->rtsp_buffer.new_frame) {
            GstBuffer* buffer = gst_sample_get_buffer(app->rtsp_buffer.sample);
            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_READ);

            update_texture(app->tex_rtsp, map.data, 800, 600);

            gst_buffer_unmap(buffer, &map);
            gst_sample_unref(app->rtsp_buffer.sample);
            app->rtsp_buffer.sample = nullptr;

            app->rtsp_buffer.new_frame = false;
        }
    }

    // Comment out the following block
    // {
    //     std::unique_lock<std::mutex> lock(app->local_buffer.mtx);
    //     if (app->local_buffer.new_frame) {
    //         GstBuffer* buffer = gst_sample_get_buffer(app->local_buffer.sample);
    //         GstMapInfo map;
    //         gst_buffer_map(buffer, &map, GST_MAP_READ);

    //         update_texture(app->tex_local, map.data, 800, 600);

    //         gst_buffer_unmap(buffer, &map);
    //         gst_sample_unref(app->local_buffer.sample);
    //         app->local_buffer.sample = nullptr;

    //         app->local_buffer.new_frame = false;
    //     }
    // }

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

    // Comment out the following lines
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, app->tex_local);
    // glUniform1i(glGetUniformLocation(app->program, "tex2"), 1);

    glUniform1f(glGetUniformLocation(app->program, "alpha"), app->alpha);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glfwSwapBuffers(app->window);
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
