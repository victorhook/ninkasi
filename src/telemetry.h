#ifndef TELEMETRY_H
#define TELEMETRY_H


typedef struct
{
    float bat_voltage;
    float bat_current;
    float bat_current_consumed;
    int8_t bat_battery_remaining; // 0-100

    float pres_abs;
    float temperature;

    int32_t gps_lat; /*< [degE7] Latitude (WGS84, EGM96 ellipsoid)*/
    int32_t gps_lon; /*< [degE7] Longitude (WGS84, EGM96 ellipsoid)*/
    int32_t gps_alt; /*< [mm] Altitude (MSL). Positive for up. Note that virtually all GPS modules provide the MSL altitude in addition to the WGS84 altitude.*/
    uint16_t gps_vel; /*< [cm/s] GPS ground speed. If unknown, set to: UINT16_MAX*/
    uint16_t gps_eph; /*<  GPS HDOP horizontal dilution of position (unitless * 100). If unknown, set to: UINT16_MAX*/
    uint16_t gps_epv; /*<  GPS VDOP vertical dilution of position (unitless * 100). If unknown, set to: UINT16_MAX*/
    uint16_t gps_cog; /*< [cdeg] Course over ground (NOT heading, but direction of movement) in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX*/
    uint8_t gps_fix_type; /*<  GPS fix type.*/
    uint8_t gps_satellites_visible; /*<  Number of satellites visible. If unknown, set to UINT8_MAX*/
    uint32_t gps_h_acc; /*< [mm] Position uncertainty.*/
    uint32_t gps_v_acc; /*< [mm] Altitude uncertainty.*/
    uint32_t gps_vel_acc; /*< [mm] Speed uncertainty.*/

    int16_t imu_xacc; /*< [mG] X acceleration*/
    int16_t imu_yacc; /*< [mG] Y acceleration*/
    int16_t imu_zacc; /*< [mG] Z acceleration*/
    int16_t imu_xgyro; /*< [mrad/s] Angular speed around X axis*/
    int16_t imu_ygyro; /*< [mrad/s] Angular speed around Y axis*/
    int16_t imu_zgyro; /*< [mrad/s] Angular speed around Z axis*/
    int16_t imu_xmag; /*< [mgauss] X Magnetic field*/
    int16_t imu_ymag; /*< [mgauss] Y Magnetic field*/
    int16_t imu_zmag; /*< [mgauss] Z Magnetic field*/
    int16_t imu_temperature; /*< [cdegC] Temperature, 0: IMU does not provide temperature values. If the IMU is at 0C it must send 1 (0.01C).*/

    float vibration_x;
    float vibration_y;
    float vibration_z;
    uint32_t imu_clipping;

    float roll;
    float pitch;
    float yaw;
    float rollspeed;
    float pitchspeed;
    float yawspeed;
    float altitude;
    float relative_altitude;
    float vx;
    float vy;
    float vz;
    float lat;
    float lon;
    float heading;
    uint16_t servo_out1;
    uint16_t servo_out2;
    uint16_t servo_out3;
    uint16_t servo_out4;
    uint16_t servo_out5;
    uint16_t servo_out6;
    uint16_t rc_channel_1;
    uint8_t rc_rssi;
}__attribute__((packed)) Telemetry;


#endif /* TELEMETRY_H */
