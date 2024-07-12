#!/usr/bin/env python3

from picamera2.encoders import H264Encoder, MJPEGEncoder, Quality
from picamera2 import Picamera2
from picamera2.outputs import FileOutput
from websocket_server import WebsocketServer, WebSocketHandler
import utils

from queue import Queue
import base64
import time
import socket
from threading import Thread
import io

IMAGE_WIDHT = 640
IMAGE_HEIGHT = 480

logger = utils.get_logger(__file__)


class ImageFrameHandler:

    def handle_frame(self, frame) -> None:
        pass


class ImageFrameOutput(FileOutput):

    def __init__(self, file: io.BufferedWriter, frame_handler: ImageFrameHandler = None) -> None:
        super().__init__(file)
        self.fps = 0
        self._fps_counter = 0
        self._t0 = time.time()
        self._frame_handler = frame_handler

    def outputframe(self, frame: bytes, keyframe: bool, timestamp: int) -> None:
        self._fps_counter += 1
        if ((time.time() - self._t0) > 1):
            self.fps = self._fps_counter
            self._fps_counter = 0
            self._t0 = time.time()
            logger.debug(f'FPS: {self.fps}')

        if self._frame_handler is not None:
            self._frame_handler.handle_frame(frame)

        #print(len(frame), keyframe, timestamp)
        #super().outputframe(frame, keyframe, timestamp)


class VideoServer(ImageFrameHandler):

    def __init__(self, port: int) -> None:
        self._ip = '0.0.0.0'
        self._port = port
        self._picam2 = Picamera2()
        self._encoder = MJPEGEncoder(10000000)
        self._out: ImageFrameOutput
        self._ws_clients = dict()
        self._img_queue = Queue(maxsize=30)

    def start(self) -> None:
        # -- Camera init -- #
        logger.info(f'Configuring camera to use resolution {IMAGE_WIDHT}x{IMAGE_HEIGHT}')
        video_config = self._picam2.create_video_configuration()
        video_config['main']['size'] = (IMAGE_WIDHT, IMAGE_HEIGHT)
        video_config['raw']['size'] = (IMAGE_WIDHT, IMAGE_HEIGHT)
        self._picam2.configure(video_config)

        # Create image output buffer and start recording
        logger.info('Starting to streaming video from camera')
        self._out = ImageFrameOutput(io.BytesIO(), frame_handler=self)
        self._picam2.start_recording(self._encoder, self._out, quality=Quality.VERY_LOW)

        # -- Server init -- #
        logger.info(f'VideoServer starting websocket server at port {self._port}')
        self.server = WebsocketServer('0.0.0.0', self._port)
        self.server.set_fn_new_client(self.ws_cb_new_client)
        self.server.set_fn_client_left(self.ws_cb_client_left)
        self.server.set_fn_message_received(self.ws_cb_message_received)

        Thread(target=self.server.run_forever, daemon=True).start()
        Thread(target=self._start_video_streaming, daemon=True).start()

        #self.run(sock)

    def handle_frame(self, frame: bytes) -> None:
        # Add image frame to TX queue
        if not self._img_queue.full():
            self._img_queue.put(frame)

    def _start_video_streaming(self) -> None:
        while True:
            frame = self._img_queue.get()
            for client in self._ws_clients.values():
                handler: WebSocketHandler = client['handler']
                encoded_data = base64.b64encode(frame).decode('utf-8')
                handler.send_message(encoded_data)
            '''
            while not self.out.connectiondead:
                time.sleep(0.1)
                self.out.fileoutput.seek(0)
                frame_data = self.out.fileoutput.read()
                if frame_data:
                    encoded_data = base64.b64encode(frame_data).decode('utf-8')
                    for client in self.ws_clients:
                        self.server.send_message(client, encoded_data)
                    self.out.fileoutput.truncate(0)

            logger.info('Done!')
            self._picam2.stop_recording()
            '''

    # -- Websocket handlers -- #
    def ws_cb_new_client(self, client: dict, server: WebsocketServer) -> None:
        # 'id': 1, 'handler': <websocket_server.websocket_server.WebSocketHandler object at 0x7f78af1d50>, 'address': ('192.168.10.204', 53532)
        try:
            logger.debug(f'Client connected: {client}')
            self._ws_clients[client['id']] = client
        except Exception as e:
            logger.error(f'Error adding new video client: {e}')

    def ws_cb_client_left(self, client: dict, server: WebsocketServer) -> None:
        id = client.get('id')
        if id in self._ws_clients:
            self._ws_clients.pop(id)
            logger.debug(f'Client disconnected: {client}')

    def ws_cb_message_received(self, client: dict, server: WebsocketServer, message) -> None:
        print(type(client), type(server), type(message), 'MSG')


if __name__ == '__main__':
    VideoServer(2347).start()
    while True:
        time.sleep(1)