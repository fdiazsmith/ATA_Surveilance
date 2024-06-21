#include "gstreamer_utils.h"
#include "opengl_utils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <opencv2/opencv.hpp>
#include "circular_buffer.h"

CircularBuffer frameBuffer(300); // Assuming 10 seconds delay at 30 fps


static GstFlowReturn new_frame_callback_rtsp(GstAppSink* appsink, gpointer user_data) {
    AppData* app = (AppData*)user_data;
    GstSample* sample = gst_app_sink_pull_sample(appsink);
    if (!sample) {
        return GST_FLOW_ERROR;
    }

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    if (buffer) {
        frameBuffer.addFrame(buffer);
    }

    gst_sample_unref(sample);
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

void init_gstreamer(AppData* app, const std::string& rtsp_url, int width, int height, bool delay_video, int video_delay) {
    gst_init(NULL, NULL);

   
    std::string rtsp_pipeline = "rtspsrc location=" + rtsp_url + " latency=0 ! "
                                "rtph264depay ! "
                                "h264parse ! "
                                "avdec_h264 ! ";
    
    if (delay_video) {
        // Calculate max-size-time based on video_delay (in nanoseconds)
        int64_t max_size_time = static_cast<int64_t>(video_delay) * 1000000000;
        rtsp_pipeline += "! queue max-size-time=" + std::to_string(max_size_time) + " ! videorate ! video/x-raw,framerate=25/1 ";
    }

    rtsp_pipeline += "videoconvert ! videoscale ! video/x-raw,format=RGB,width=" + std::to_string(width) + ",height=" + std::to_string(height) + " ! appsink name=appsink_rtsp sync=false";
    // rtsp_pipeline += "videoconvert ! videoscale ! video/x-raw,format=RGB,width=" + std::to_string(width) + ",height=" + std::to_string(height) + " ! appsink name=appsink_rtsp";
 

    // print the rpsdpipeline
    printf("RTSP pipeline: %s\n", rtsp_pipeline.c_str());
    app->pipeline_rtsp = gst_parse_launch(rtsp_pipeline.c_str(), NULL);
    app->appsink_rtsp = gst_bin_get_by_name(GST_BIN(app->pipeline_rtsp), "appsink_rtsp");

    // Disable buffering on appsink
    if (delay_video) {
        // g_object_set(G_OBJECT(app->appsink_rtsp), "enable-last-sample", FALSE, NULL);
        printf("Disabling last sample\n");
    }else{
         g_object_set(G_OBJECT(app->appsink_rtsp), "drop", TRUE, "sync", FALSE, NULL);
    }
    

    GstAppSinkCallbacks callbacks_rtsp = { NULL, NULL, new_frame_callback_rtsp };
    gst_app_sink_set_callbacks(GST_APP_SINK(app->appsink_rtsp), &callbacks_rtsp, app, NULL);

    GstBus* bus_rtsp = gst_pipeline_get_bus(GST_PIPELINE(app->pipeline_rtsp));
    gst_bus_add_watch(bus_rtsp, bus_call, g_main_loop_new(NULL, FALSE));
    gst_object_unref(bus_rtsp);

    gst_element_set_state(app->pipeline_rtsp, GST_STATE_PLAYING);
}

void load_texture_from_buffer(AppData* app, size_t delay_frames) {
    GstBuffer* buffer = frameBuffer.getFrame(delay_frames);
    if (buffer) {
        GstMapInfo map;
        if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            // Convert GstBuffer to an OpenCV Mat
            cv::Mat img(cv::Size(app->texture_width, app->texture_height), CV_8UC3, (char*)map.data, cv::Mat::AUTO_STEP);

            // Process the image if needed
            // Example: Convert to another format, apply filters, etc.

            // Load the Mat as a texture
            load_texture_from_mat(img);

            gst_buffer_unmap(buffer, &map);
        }
        gst_buffer_unref(buffer);
    }
}

void load_texture_from_mat(const cv::Mat& img) {
    // Generate texture ID
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load the image data into the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void render(AppData* app) {
    {
        std::unique_lock<std::mutex> lock(app->rtsp_buffer.mtx);
        if (app->rtsp_buffer.new_frame) {
            GstBuffer* buffer = gst_sample_get_buffer(app->rtsp_buffer.sample);
            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_READ);

            update_texture(app->tex_rtsp, map.data, app->texture_width, app->texture_height);

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
