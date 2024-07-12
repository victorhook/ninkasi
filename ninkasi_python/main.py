#!/usr/bin/env python3

from ast import Set
import sys
import os
from utils import get_logger, millis, micros
import time
from threading import Thread
from mavcom import MavCom
from settings import Settings
from video import VideoServer


FRAMERATE_HZ = 50
logger = get_logger(__file__)


def main_loop() -> None:
    period_us = (1 / FRAMERATE_HZ) * 1000000
    next_period = micros()

    while True:
        next_period += period_us

        sleep_duration = next_period - micros()
        if sleep_duration > 0:
            time.sleep(sleep_duration / 1000000)


def start_thread(target: callable, name: str, as_daemon: bool = True, *args) -> None:
    logger.info(f'Starting thread: {name}')
    Thread(target=target, name=name, args=args, daemon=as_daemon).start()


if __name__ == '__main__':
    settings = Settings()
    mavcom = MavCom(settings.serial_port, settings.serial_baud)
    video_server = VideoServer(settings.video_server_port)

    # Start threads
    start_thread(mavcom.start, 'MavCom')
    start_thread(video_server.start, 'VideoServer')

    main_loop()
