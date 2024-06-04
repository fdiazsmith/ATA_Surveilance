import cv2


class CameraStream:
    def __init__(self, rtsp_url):
        self.rtsp_url = rtsp_url
        self.cap = None

    def main(self):
        # Open a connection to the IP camera
        self.cap = cv2.VideoCapture(self.rtsp_url)

        # Check if the connection was successful
        if not self.cap.isOpened():
            print("Error: Unable to open video stream")
            return

        # Create a named window
        window_name = "Reolink RLC-510A Stream"
        cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(window_name, 800, 600)  # Optional: Resize window

        while True:
            # Capture frame-by-frame
            ret, frame = self.cap.read()

            if not ret:
                print("Error: Unable to read frame")
                break

            # Display the resulting frame
            cv2.imshow(window_name, frame)

            # Press 'q' to quit
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    def release(self):
        # Release the capture and close windows
        if self.cap is not None:
            self.cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    try:
        camera = CameraStream('rtsp://admin:anna.landa85@10.0.0.26/Preview_01_main')
        camera.main()
    except KeyboardInterrupt:
        camera.release()
        print("Keyboard interrupt detected. Exiting...")
