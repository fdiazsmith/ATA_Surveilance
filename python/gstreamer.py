# opencv with gstreamer and ffmpeg
import cv2

import cv2

# Replace with your RTSP URL
RTSP_URL = "rtsp://admin:anna.landa85@10.0.0.26:554/Preview_01_main"

def main():
    # Define the GStreamer pipeline
    gst_pipeline = (
        f"rtspsrc location={RTSP_URL} latency=0 ! "
        "rtph264depay ! "
        "h264parse ! "
        "avdec_h264 ! "
        "videoconvert ! "
        "appsink"
    )

    # Open a connection to the RTSP stream
    cap = cv2.VideoCapture(gst_pipeline, cv2.CAP_GSTREAMER)

    # Check if the connection was successful
    if not cap.isOpened():
        print("Error: Unable to open video stream")
        return

    # Create a named window
    window_name = "RTSP Stream"
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.resizeWindow(window_name, 800, 600)  # Optional: Resize window

    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()

        if not ret:
            print("Error: Unable to read frame")
            break

        # Manipulate the frame (e.g., convert to grayscale)
        gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Display the resulting frame
        cv2.imshow(window_name, gray_frame)

        # Press 'q' to quit
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Release the capture and close windows
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
