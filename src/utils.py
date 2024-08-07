import logging
import time

__all__ = [
    'logger',
    'millis',
    'micros'
]


MAIN_LOG_FILE = './ninkasi.out'


class CustomFormatter(logging.Formatter):
    def format(self, record):
        # Align the log level to a fixed width of 8 characters
        record.levelname = '{:<7}'.format(record.levelname)
        return super().format(record)


LOG_FORMATTER = CustomFormatter(
    '[%(levelname)s] %(asctime)s.%(msecs)03d - %(name)s: %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
LOG_DEFAULT_LEVEL = logging.DEBUG

t0 = time.time()

loggers = {}

# Stream handler (stdout/stderr)
handlers = [
    logging.StreamHandler(),
    logging.FileHandler(MAIN_LOG_FILE, 'a')
]

for handler in handlers:
    handler.setFormatter(LOG_FORMATTER)
    handler.setLevel(LOG_DEFAULT_LEVEL)


def get_logger(name: str) -> logging.Logger:
    global loggers
    if name not in loggers:
        logger = logging.getLogger(name)

        # Stream handler (stdout/stderr)
        handlers = [
            logging.StreamHandler(),
            logging.FileHandler(MAIN_LOG_FILE, 'a')
        ]

        for handler in handlers:
            handler.setFormatter(LOG_FORMATTER)
            handler.setLevel(LOG_DEFAULT_LEVEL)

        logger.handlers = handlers
        logger.setLevel(LOG_DEFAULT_LEVEL)
        loggers[name] = logger

    return loggers[name]

def millis() -> int:
    ''' Returns the time in milliseconds since program start. '''
    return int((time.time() - t0) * 1000)

def micros() -> int:
    ''' Returns the time in microseconds since program start. '''
    return int((time.time() - t0) * 1000000)


from abc import abstractmethod
from threading import Thread, Event
import socket
import fcntl
import struct

class TcpServer:

    _client_id = 0

    def __init__(self,
                 port: int,
                 update_thread: bool = True,
                 update_thread_period_ms: int = 100,
                 handle_client_in_thread: bool = True
                 ) -> None:
        self._port = port
        self._handle_client_in_thread = handle_client_in_thread
        self._update_thread = update_thread
        self._update_thread_period_ms = update_thread_period_ms

        self._stop_flag = Event()
        self._name = self.__class__.__name__
        self.log = get_logger(self._name)
        self._sock: socket.socket

    @abstractmethod
    def handle_client(self, client: socket.socket, client_id: int) -> None:
        pass

    @abstractmethod
    def update(self) -> None:
        pass

    def start(self) -> None:
        self.log.info(f'Starting at port {self._port}')

        self._stop_flag.clear()

        Thread(target=self._run, daemon=True).start()

        if self._update_thread:
            Thread(target=self._updater, name=f'{self._name}_update', daemon=True).start()

    def stop(self) -> None:
        self._stop_flag.set()

    def is_running(self) -> bool:
        return not self._stop_flag.is_set()

    def available_bytes(self, sock: socket.socket) -> int:
        ''' Returns the number of bytes available of the given socket'''
        FIONREAD = 0x541B
        # Perform ioctl system call to get the number of bytes available for reading
        pending = struct.unpack('I', fcntl.ioctl(sock, FIONREAD, struct.pack('I', 0)))[0]
        return pending

    def _updater(self) -> None:
        next_update = micros()

        while self.is_running():

            self.update()

            time_left_us = next_update - micros()
            if time_left_us > 0:
                time.sleep(time_left_us / 1000000)

            next_update += self._update_thread_period_ms


    def _run(self) -> None:
        self.log.debug(f'Thread started')

        # Create TCP socket and start listening
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(('0.0.0.0', self._port))
        self._sock.listen(0)

        # Run forever and handle incoming connections
        while self.is_running():
            client, addr = self._sock.accept()
            self._client_id += 1
            self.log.debug(f'New client #{self._client_id} from {addr}')

            if self._handle_client_in_thread:
                Thread(target=self.handle_client,
                       name=f'{self._name}_client#{self._client_id}',
                       args=(client, self._client_id),
                       daemon=True
                       ).start()
            else:
                self.handle_client(client, self._client_id)

        self.log.debug(f'Thread stopped')
