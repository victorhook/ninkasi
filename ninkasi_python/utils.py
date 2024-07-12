import logging
import time

__all__ = [
    'logger',
    'millis',
    'micros'
]

t0 = time.time()

loggers = {}


def get_logger(name: str) -> logging.Logger:
    global loggers
    if name not in loggers:
        logger = logging.getLogger(name)
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter('[%(levelname)s] %(asctime)s: %(message)s'))

        handler.setLevel(logging.DEBUG)
        logger.setLevel(logging.DEBUG)

        logger.handlers = [handler]
        loggers[name] = logger

    return loggers[name]

def millis() -> int:
    ''' Returns the time in milliseconds since program start. '''
    return int((time.time() - t0) * 1000)

def micros() -> int:
    ''' Returns the time in microseconds since program start. '''
    return int((time.time() - t0) * 1000000)