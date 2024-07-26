from pymavlink.dialects.v20.ardupilotmega import MAVLink, MAVLink_message
from pymavlink.mavutil import mavlink_connection

import utils
from serial import Serial
from threading import Thread, Event
from typing import List, Dict

logger = utils.get_logger(__name__)


class MavlinkSubscriber:

    def handle_message(self, msg: MAVLink_message) -> None:
        pass

class MavCom:

    def __init__(self, port: str, baud: int) -> None:
        self._port = port
        self._baud = baud
        self._con = None
        self._mav: MAVLink
        self._serial: Serial
        self._stop_flag = Event()
        self._subscribers: Dict[str, List[MavlinkSubscriber]] = dict()

    def subscribe(self, subscriber: MavlinkSubscriber, msg_type: str = 'all') -> None:
        if msg_type not in self._subscribers:
            self._subscribers[msg_type] = []

        self._subscribers[msg_type].append(subscriber)

    def start(self) -> None:
        logger.info(f'Connecting to serial port {self._port}, baudrate: {self._baud}')
        self._serial = Serial(self._port, self._baud)
        self._mav = MAVLink (self._serial)
        self._stop_flag.clear()
        Thread(target=self._reader, name='MavCom RX', daemon=True).start()
        Thread(target=self._writer, name='MavCom TX', daemon=True).start()
        logger.info('Connected!')

    def stop(self) -> None:
        self._stop_flag.set()

    def _handle_mavlink_message(self, msg: MAVLink_message) -> None:
        msg_type = msg.get_type()
        handled = False
        for sub in self._subscribers.get(msg_type, []) + self._subscribers.get('all', []):
            handled = True
            sub.handle_message(msg)

        if not handled:
            pass
            #print(f'No handler found for {msg}')

    def _reader(self) -> None:
        logger.debug(f'Reader thread started')
        while not self._stop_flag.is_set():
            byte = self._serial.read(1)

            msg = self._mav.parse_char(byte)
            if msg:
                self._handle_mavlink_message(msg)

        logger.debug(f'Reader thread ended')

    def _writer(self) -> None:
        logger.debug(f'Writer thread started')
        while not self._stop_flag.is_set():
            pass
        logger.debug(f'Writer thread ended')
