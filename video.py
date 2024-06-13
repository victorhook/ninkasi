#!/usr/bin/env python3

from picamera2.encoders import H264Encoder, MJPEGEncoder, Quality
from picamera2 import Picamera2
from picamera2.outputs import FileOutput

import time
import socket
from threading import Thread
import io

VIDEO_SERVER_TCP_PORT = 2348
IMAGE_WIDHT = 640
IMAGE_HEIGHT = 480


class ImageFrameHandler:

    def handle_frame(self, frame):
        pass


class ImageFrameOutput(FileOutput):

    def __init__(self, file: io.BufferedWriter, frame_handler: ImageFrameHandler = None) -> None:
        super().__init__(file)
        self.fps_counter = 0
        self.t0 = time.time()
        self.frame_handler = frame_handler

    def outputframe(self, frame, keyframe=True, timestamp=None) -> None:
        self.fps_counter += 1
        if ((time.time() - self.t0) > 1):
            self.fps = self.fps_counter
            self.fps_counter = 0
            self.t0 = time.time()

        if self.frame_handler is not None:
            self.frame_handler.handle_frame(frame)

        super().outputframe(frame, keyframe, timestamp)


class VideoServer:

    def __init__(self) -> None:
        self.ip = '0.0.0.0'
        self.port = VIDEO_SERVER_TCP_PORT
        self.picam2 = Picamera2()
        self.encoder = MJPEGEncoder(10000000)
        self.out: ImageFrameOutput

    def start(self) -> None:
        # Server init
        print(f'[INFO] VideoServer starting at TCP port {self.port}')
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((self.ip, self.port))
        sock.listen(0)

        # Camera init
        print(f'[INFO] Configuring camera to use resolution {IMAGE_WIDHT}x{IMAGE_HEIGHT}')
        video_config = self.picam2.create_video_configuration()
        video_config['main']['size'] = (IMAGE_WIDHT, IMAGE_HEIGHT)
        video_config['raw']['size'] = (IMAGE_WIDHT, IMAGE_HEIGHT)
        self.picam2.configure(video_config)

        self.run(sock)

    def run(self, sock: socket.socket) -> None:
        while True:
            try:
                client, addr = sock.accept()
                ip, port = addr
                print(f'[INFO] New TCP connection from {ip}:{port}')

                Thread(target=self._start_video_streaming, args=(client,)).start()

            except Exception as e:
                print(f'[ERROR] {e}')

    def _start_video_streaming(self, client: socket.socket) -> None:
        if self.encoder.running:
            print('[WARNING] A client is already connected. Closing that and using new')
            try:
                self.picam2.stop_recording()
                time.sleep(0.5)
            except Exception as e:
                print(f'[ERROR] Closing previous client {e}')

        print('[INFO] Starting to stream video')
        self.out = ImageFrameOutput(client.makefile("wb"), frame_handler=None)
        self.picam2.start_recording(self.encoder, self.out, quality=Quality.VERY_LOW)

        while not self.out.connectiondead:
            time.sleep(1)

        print('Done!')
        self.picam2.stop_recording()


if __name__ == '__main__':
    VideoServer().start()