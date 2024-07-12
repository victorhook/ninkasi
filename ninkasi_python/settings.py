from dataclasses import dataclass


@dataclass
class Settings:
    serial_port = '/dev/serial0'
    serial_baud = 921600
    command_server_port = 2345
    telemetry_server_port = 4367
    video_server_port = 2347
    log_server_port = 2348