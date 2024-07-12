from pymavlink.dialects.v20.ardupilotmega import MAVLink
from pymavlink.mavutil import mavlink_connection


class MavCom:

    def __init__(self, port: str, baud: int) -> None:
        self.port = port
        self.baud = baud
        self._con = None

    def start(self) -> None:
        self._con = mavlink_connection(self.port, self.baud)
        print('Connected!')