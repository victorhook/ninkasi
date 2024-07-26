from dataclasses import dataclass
import struct


@dataclass
class Telemetry:
    uptime: float = 0

    bat_current: float = 0
    bat_voltage: float = 0
    bat_current_consumed: float = 0
    bat_battery_remaining: float = 0

    pres_abs: float = 0
    temperature: float = 0

    gps_lat: float = 0 # [degE7] Latitude (WGS84, EGM96 ellipsoid)
    gps_lon: float = 0 # [degE7] Longitude (WGS84, EGM96 ellipsoid)
    gps_alt: float = 0 # [mm] Altitude (MSL). Positive for up. Note that virtually all GPS modules provide the MSL altitude in addition to the WGS84 altitude.
    gps_vel: float = 0 # [cm/s] GPS ground speed. If unknown, set to: UINT16_MAX
    gps_eph: float = 0 #  GPS HDOP horizontal dilution of position (unitless * 100). If unknown, set to: UINT16_MAX
    gps_epv: float = 0 #  GPS VDOP vertical dilution of position (unitless * 100). If unknown, set to: UINT16_MAX
    gps_cog: float = 0 # [cdeg] Course over ground (NOT heading, but direction of movement) in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX
    gps_fix_type: float = 0 #  GPS fix type.
    gps_satellites_visible: float = 0 #  Number of satellites visible. If unknown, set to UINT8_MAX
    gps_h_acc: float = 0 # [mm] Position uncertainty.
    gps_v_acc: float = 0 # [mm] Altitude uncertainty.
    gps_vel_acc: float = 0 # [mm] Speed uncertainty.

    imu_xacc: float = 0 # [mG] X acceleration
    imu_yacc: float = 0 # [mG] Y acceleration
    imu_zacc: float = 0 # [mG] Z acceleration
    imu_xgyro: float = 0 # [mrad/s] Angular speed around X axis
    imu_ygyro: float = 0 # [mrad/s] Angular speed around Y axis
    imu_zgyro: float = 0 # [mrad/s] Angular speed around Z axis
    imu_xmag: float = 0 # [mgauss] X Magnetic field
    imu_ymag: float = 0 # [mgauss] Y Magnetic field
    imu_zmag: float = 0 # [mgauss] Z Magnetic field
    imu_temperature: float = 0 # [cdegC] Temperature, 0: IMU does not provide temperature values. If the IMU is at 0C it must send 1 (0.01C).

    vibration_x: float = 0
    vibration_y: float = 0
    vibration_z: float = 0
    imu_clipping: float = 0

    roll: float = 0
    pitch: float = 0
    yaw: float = 0
    rollspeed: float = 0
    pitchspeed: float = 0
    yawspeed: float = 0
    altitude: float = 0
    relative_altitude: float = 0
    vx: float = 0
    vy: float = 0
    vz: float = 0
    lat: float = 0
    lon: float = 0
    heading: float = 0

    rangefinder_distance: float = 0 # In meters

    flow_x: float = 0
    flow_y: float = 0

    servo_out1: float = 0
    servo_out2: float = 0
    servo_out3: float = 0
    servo_out4: float = 0
    servo_out5: float = 0
    servo_out6: float = 0
    rc_channel_1: float = 0
    rc_rssi: float = 0


FMT = ''.join('f' for i in range(len(Telemetry.__dataclass_fields__)))


def telem_to_bytes(telemetry: Telemetry) -> bytes:
    return struct.pack(FMT, *[getattr(telemetry, attr) for attr in telemetry.__dataclass_fields__])
