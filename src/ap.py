from telemetry import Telemetry
from utils import get_logger, millis
from mavcom import MavCom, MavlinkSubscriber, MAVLink_message, ardupilotmega
from mission.mission import Mission
from mission.idle import IdleMission


logger = get_logger(__name__)


class AP(MavlinkSubscriber):

    def __init__(self, mavcom: MavCom) -> None:
        super().__init__()
        self.telemetry = Telemetry()
        self.mavcom = mavcom
        self.mavcom.subscribe(self)
        self._last_heartbeat = 0
        self.mission: Mission = IdleMission()

    def setup(self) -> None:
        self._setup_message_intervals()

    def handle_message(self, msg: MAVLink_message) -> None:
        msg_type = msg.get_type()
        if msg_type == 'STATUSTEXT':
            logger.debug(f'{msg}')
        elif msg_type == 'HEARTBEAT':
            pass
        logger.debug(f'AP: [{msg_type}]')

    def update(self) -> None:
        '''
        Should be called from main loop
        '''

        # Send mavlink heartbeat and periodic messages
        if (millis() - self._last_heartbeat) > 1000:
            self.mavcom.send_message(self.mavcom._mav.heartbeat_encode(ardupilotmega.MAV_TYPE_ONBOARD_CONTROLLER, ardupilotmega.MAV_AUTOPILOT_INVALID, 0, 0, 0))
            self._last_heartbeat = millis()

        # We only run mission logic if we're not in guided


    def _setup_message_intervals(self) -> None:
        self.mavcom.set_message_interval(ardupilotmega.MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 10)