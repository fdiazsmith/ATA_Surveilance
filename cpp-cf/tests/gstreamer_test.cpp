// g++ -o gstreamer_test gstreamer_test.cpp `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0`

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <iostream>

struct AppData {
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

    app->pipeline = gst_parse_launch("filesrc location=assets/static.mp4 ! decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB,width=800,height=600 ! appsink name=appsink", NULL);
    app->appsink = gst_bin_get_by_name(GST_BIN(app->pipeline), "appsink");

    GstAppSinkCallbacks callbacks = { NULL, NULL, new_frame_callback };
    gst_app_sink_set_callbacks(GST_APP_SINK(app->appsink), &callbacks, app, NULL);

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(app->pipeline));
    gst_bus_add_watch(bus, bus_call, g_main_loop_new(NULL, FALSE));
    gst_object_unref(bus);

    gst_element_set_state(app->pipeline, GST_STATE_PLAYING);
}

int main() {
    AppData app = {};
    app.new_frame = false;

    init_gstreamer(&app);

    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    gst_element_set_state(app.pipeline, GST_STATE_NULL);
    gst_object_unref(app.pipeline);

    return 0;
}
