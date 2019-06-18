
from threading import Thread
import cv2
import time


class CameraStream:
    def __init__(self, src=0, fps=1, file_prefix=None):
        # initialize the video camera stream and read the first frame
        # from the stream
        self.stream = cv2.VideoCapture(src)
        (self.grabbed, self.frame) = self.stream.read()

        # initialize the variable used to indicate if the thread should
        # be stopped
        self.stopped = False

        # set fps here
        self.fps = fps
        self.sleep_time = 1 / self.fps

        self.file_prefix = file_prefix
        self.image_index = 0

        self.timestart = time.time()

    def start(self):
        # start the thread to read frames from the video stream
        Thread(target=self.update, args=()).start()
        return self

    def update(self):
        # keep looping infinitely until the thread is stopped
        while True:
            time.sleep(self.sleep_time)
            # if the thread indicator variable is set, stop the thread
            if self.stopped:
                return

            # otherwise, read the next frame from the stream
            # self.grabbed is a bool
            # self.frame is the image, probably an image object.
            # (self.grabbed, self.frame) = self.stream.read()
            (grabbed, frame) = self.stream.read()
            cv2.imwrite(
                '{}_{}.jpg'.format(self.file_prefix, self.image_index),
                frame)
            self.image_index += 1

    def read(self):
        # return the frame most recently read
        return self.frame

    def stop(self):
        # indicate that the thread should be stopped
        self.stopped = True
