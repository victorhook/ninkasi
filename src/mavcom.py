from abc import abstractmethod
from queue import Queue
from pymavlink.dialects.v20.ardupilotmega import MAVLink, MAVLink_message, MAVError
from pymavlink.dialects.v20 import ardupilotmega

import utils
from serial import Serial
from threading import Thread, Event
from typing import List, Dict, Union


AP_SYSTEM_ID = 1
AP_COMPONENT_ID = 1
NINKASI_SYSTEM_ID = 42
NINKASI_COMPONENT_ID = ardupilotmega.MAV_COMP_ID_ONBOARD_COMPUTER


logger = utils.get_logger(__name__)

class MavlinkSubscriber:

    id = 0

    def __init__(self) -> None:
        MavlinkSubscriber.id += 1

    @abstractmethod
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
        self._subscribers: Dict[str, Dict[int, MavlinkSubscriber]] = dict()
        self._tx = Queue() # Queue[MAVLink_message]
        self._parse_errors = 0

    def subscribe(self, subscriber: MavlinkSubscriber, msg_type: str = 'all') -> None:
        logger.debug(f'New subscriber #{subscriber.id} to msg_type {msg_type}')
        if msg_type not in self._subscribers:
            self._subscribers[msg_type] = dict()

        self._subscribers[msg_type][subscriber.id] = subscriber

    def unsubscribe(self, subscriber: MavlinkSubscriber) -> None:
        for msg_type, subs in self._subscribers.items():
            if subscriber.id in subs:
                logger.debug(f'Removing subscriber #{subscriber.id} from msg_type {msg_type}')
                subs.pop(subscriber.id)
                return

    def start(self) -> None:
        logger.info(f'Connecting to serial port {self._port}, baudrate: {self._baud}')
        self._serial = Serial(self._port, self._baud)
        self._mav = MAVLink(self._serial, srcSystem=NINKASI_SYSTEM_ID, srcComponent=NINKASI_COMPONENT_ID)
        self._stop_flag.clear()
        Thread(target=self._reader, name='MavCom RX', daemon=True).start()
        Thread(target=self._writer, name='MavCom TX', daemon=True).start()
        logger.info('Connected!')

    def stop(self) -> None:
        self._stop_flag.set()

    def send_message(self, msg: MAVLink_message) -> None:
        self._tx.put(msg)

    def _handle_mavlink_message(self, msg: MAVLink_message) -> None:
        msg_type = msg.get_type()
        handlers = 0

        for sub in self._subscribers.get(msg_type, {}).values():
            sub.handle_message(msg)
            handlers += 1

        for sub in self._subscribers.get('all', {}).values():
            sub.handle_message(msg)
            handlers += 1

        if handlers == 0:
            logger.debug(f'No handler found for {msg}')

    def _reader(self) -> None:
        logger.debug(f'Reader thread started')
        while not self._stop_flag.is_set():
            byte = self._serial.read(1)

            try:
                msg = self._mav.parse_char(byte)
                if msg:
                    # OF module src comp: 88, sys: 11
                    if msg.get_srcComponent() == 88:
                        pass
                    else:
                        self._handle_mavlink_message(msg)
                        #logger.debug(f'RX <- [{msg.get_srcComponent():3}:{msg.get_srcSystem():3}]: {msg.get_type()} ({msg.get_seq()}) {msg}')
            except MAVError:
                self._parse_errors += 1

        logger.debug(f'Reader thread ended')

    def _writer(self) -> None:
        logger.debug(f'Writer thread started')

        while not self._stop_flag.is_set():
            try:
                msg: MAVLink_message = self._tx.get()

                # Apply correct sys and comp id if needed
                if msg.get_srcSystem() == 0:
                    msg._header.srcSystem = NINKASI_SYSTEM_ID
                    msg._header.srcComponent = NINKASI_COMPONENT_ID

                #logger.debug(f'TX -> [{msg.get_srcComponent():3}:{msg.get_srcSystem():3}]: {msg.get_type()} (seq: {msg.get_seq()})')
                self._mav.send(msg)
            except Exception as e:
                logger.warn(f'Error when writing mavlink data to AP: {e}')

        logger.debug(f'Writer thread ended')
