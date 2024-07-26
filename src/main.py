#!/usr/bin/env python3

from ast import Set
import sys
import os

import time
from threading import Thread

from ap import AP
from telemetry import Telemetry
from utils import get_logger, millis, micros
from mavcom import MavCom
from settings import Settings
from video import VideoServer
from telemetry_server import TelemetryServer


FRAMERATE_HZ = 50
logger = get_logger(__name__)



if __name__ == '__main__':
    settings = Settings()
    ap = AP()
    mavcom = MavCom(settings.serial_port, settings.serial_baud)
    video_server = VideoServer(settings.video_server_port)
    telemetry_server = TelemetryServer(settings.telemetry_server_port, ap.telemetry)

    # Start servers/services
    mavcom.start()
    video_server.start()
    telemetry_server.start()


    # Main loop starts here
    period_us = (1 / FRAMERATE_HZ) * 1000000
    next_period = micros()

    while True:
        next_period += period_us

        ap.update()
        telemetry_server.update()

        sleep_duration = next_period - micros()
        if sleep_duration > 0:
            time.sleep(sleep_duration / 1000000)