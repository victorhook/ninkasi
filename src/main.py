#!/usr/bin/env python3

import sys
import os
import time

from ap import AP
from utils import get_logger, millis, micros
from mavcom import MavCom
from mavproxy import MavProxy
from settings import Settings
from telemetry_server import TelemetryServer


FRAMERATE_HZ = 50
logger = get_logger(__name__)



if __name__ == '__main__':
    env = os.environ.get('NINKASI_ENV')

    settings = Settings()

    mavcom = MavCom(settings.serial_port, settings.serial_baud)
    mavproxy = MavProxy(settings.mavproxy_server_port, mavcom)

    ap = AP(mavcom)
    telemetry_server = TelemetryServer(settings.telemetry_server_port, ap.telemetry)


    # Local / Onboard specific
    if env == 'local':
        pass
    else:
        from video import VideoServer
        video_server = VideoServer(settings.video_server_port)

    # Start servers/services
    telemetry_server.start()
    mavcom.start()
    mavproxy.start()

    # Local / Onboard specific
    if env == 'local':
        pass
    else:
        video_server.start()


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