// g++ -o gstreamer_minimal gstreamer_minimal.cpp `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0`

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <iostream>

// Callback to handle new frames from the appsink
static GstFlowReturn new_frame_callback(GstAppSink* sink, gpointer data) {
    GstSample* sample = gst_app_sink_pull_sample(sink);
    if (!sample) {
        std::cerr << "Failed to pull sample from app sink" << std::endl;
        return GST_FLOW_ERROR;
    }

    GstCaps* caps = gst_sample_get_caps(sample);
    if (caps) {
        int width, height;
        GstStructure* structure = gst_caps_get_structure(caps, 0);
        gst_structure_get_int(structure, "width", &width);
        gst_structure_get_int(structure, "height", &height);
        std::cout << "Video dimensions: " << width << "x" << height << std::endl;

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo map;
        if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            std::cerr << "Failed to map buffer" << std::endl;
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        size_t expected_size = width * height * 3; // Assuming RGB format
        std::cout << "Buffer size: " << map.size << ", Expected size: " << expected_size << std::endl;

        if (map.size != expected_size) {
            std::cerr << "Error: Buffer size does not match expected size." << std::endl;
            gst_buffer_unmap(buffer, &map);
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        // Process the buffer data here

        gst_buffer_unmap(buffer, &map);
    } else {
        std::cerr << "Failed to get caps from sample" << std::endl;
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

void init_gstreamer_pipeline(const char* pipeline_str, GstAppSink** appsink) {
    GstElement* pipeline = gst_parse_launch(pipeline_str, NULL);
    if (!pipeline) {
        std::cerr << "Failed to create pipeline" << std::endl;
        exit(1);
    }

    *appsink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipeline), "appsink"));
    if (!*appsink) {
        std::cerr << "Failed to get appsink" << std::endl;
        exit(1);
    }

    GstAppSinkCallbacks callbacks = { NULL, NULL, new_frame_callback };
    gst_app_sink_set_callbacks(*appsink, &callbacks, NULL, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    const char* rtsp_pipeline = "rtspsrc location=rtsp://admin:anna.landa85@10.0.0.26:554/Preview_01_main ! decodebin ! videoconvert ! video/x-raw,format=RGB,width=2304,height=1296 ! appsink name=appsink";
    const char* local_pipeline = "filesrc location=assets/static.mov ! decodebin ! videoconvert ! video/x-raw,format=RGB,width=2304,height=1296 ! appsink name=appsink";

    GstAppSink* rtsp_sink;
    GstAppSink* local_sink;

    init_gstreamer_pipeline(rtsp_pipeline, &rtsp_sink);
    init_gstreamer_pipeline(local_pipeline, &local_sink);

    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();

    return 0;
}
