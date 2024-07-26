from telemetry import Telemetry


class AP:

    def __init__(self) -> None:
        self.telemetry = Telemetry()

    def update(self) -> None:
        '''
        Should be called from main loop
        '''
