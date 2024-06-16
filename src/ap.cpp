#include "ap.h"
#include "utils.h"



AP::AP(const char* serial_port, const int baudrate) :
mavcom(serial_port, baudrate),
m_ctrl_state(ControlState::IDLE)
{
    m_uptime_t0 = millis();
}

AP::~AP()
{

}

bool AP::init()
{
    bool mavcom_ok = mavcom.init();
    if (!mavcom_ok)
    {
        printf("Mavcom failed initialization, this is serious issue!\n");
        return false;
    }
    // Send initial requests
    mavcom.request_message(MAVLINK_MSG_ID_AUTOPILOT_VERSION);

    // Init: AUTOPILOT_VERSION
    mavcom.request_data_stream(MAVLINK_MSG_ID_GPS_RAW_INT,         100);
    mavcom.request_data_stream(MAVLINK_MSG_ID_SCALED_IMU,          100);
    mavcom.request_data_stream(MAVLINK_MSG_ID_SCALED_PRESSURE,     100);
    mavcom.request_data_stream(MAVLINK_MSG_ID_LOCAL_POSITION_NED,  100);
    mavcom.request_data_stream(MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 100);
    mavcom.request_data_stream(MAVLINK_MSG_ID_ATTITUDE,            100);
    mavcom.request_data_stream(MAVLINK_MSG_ID_BATTERY_STATUS,      100);
    mavcom.request_data_stream(MAVLINK_MSG_ID_VIBRATION,           100);
    mavcom.request_data_stream(MAVLINK_MSG_ID_RC_CHANNELS,         100);

    mavcom.request_data_stream(MAVLINK_MSG_ID_SERVO_OUTPUT_RAW,    200);

    mavcom.request_data_stream(MAVLINK_MSG_ID_SYS_STATUS,          1000);
    mavcom.request_data_stream(MAVLINK_MSG_ID_DISTANCE_SENSOR,     1000);
    mavcom.request_data_stream(MAVLINK_MSG_ID_RADIO_STATUS,        1000);

    mavcom.request_data_stream(MAVLINK_MSG_ID_ESC_STATUS,          1000);
    mavcom.request_data_stream(MAVLINK_MSG_ID_OPTICAL_FLOW,        1000);

    //mavcom.start_message_request_streams();
    return true;
}

void AP::update()
{
    if (frame_number % 25 == 0)
    {
        mavcom.send_heartbeat();
    }
    if (frame_number % 5 == 0)
    {
        send_control_signal();
    }

    telemetry.uptime = millis() - m_uptime_t0;
}

void AP::set_control_state(const ControlState ctrl_state)
{
    m_ctrl_state = ctrl_state;
}

void AP::send_control_signal()
{
    switch (m_ctrl_state)
    {
        case IDLE:
            // During idle we don't send any control signals to AP
            break;
    }
}

// Mavlink handling

void AP::handle_mavlink_message(const mavlink_message_t& msg, const mavlink_status_t& status)
{
    //printf("Mavlink message: %d\n", msg.msgid);
    switch(msg.msgid)
    {
        case MAVLINK_MSG_ID_HEARTBEAT:
            mav_handle_heartbeat(msg);
            break;
        case MAVLINK_MSG_ID_SYS_STATUS:
            mav_handle_sys_status(msg);
            break;
        case MAVLINK_MSG_ID_AUTOPILOT_VERSION:
            mav_handle_autopilot_version(msg);
            break;
        case MAVLINK_MSG_ID_STATUSTEXT:
            mav_handle_statustext(msg);
            break;
        case MAVLINK_MSG_ID_COMMAND_ACK:
            mav_handle_command_ack(msg);
            break;
        case MAVLINK_MSG_ID_PARAM_VALUE:
            mav_handle_param_value(msg);
            break;

        case MAVLINK_MSG_ID_DISTANCE_SENSOR:
            mav_handle_sys_status(msg);
            break;
        case MAVLINK_MSG_ID_RADIO_STATUS:
            mav_handle_radio_status(msg);
            break;
        case MAVLINK_MSG_ID_GPS_RAW_INT:
            mav_handle_gps_raw_int(msg);
            break;
        case MAVLINK_MSG_ID_SCALED_IMU:
            mav_handle_scaled_imu(msg);
            break;
        case MAVLINK_MSG_ID_SCALED_PRESSURE:
            mav_handle_scaled_pressure(msg);
            break;
        case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
            mav_handle_local_position_ned(msg);
            break;
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
            mav_handle_global_position_int(msg);
            break;
        case MAVLINK_MSG_ID_RC_CHANNELS:
            mav_handle_rc_channels(msg);
            break;
        case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:
            mav_handle_servo_output_raw(msg);
            break;
        case MAVLINK_MSG_ID_BATTERY_STATUS:
            mav_handle_battery_status(msg);
            break;
        case MAVLINK_MSG_ID_ATTITUDE:
            mav_handle_attitude(msg);
            break;
        case MAVLINK_MSG_ID_VIBRATION:
            mav_handle_vibration(msg);
            break;
        case MAVLINK_MSG_ID_ESC_STATUS:
            mav_handle_esc_status(msg);
            break;
        case MAVLINK_MSG_ID_OPTICAL_FLOW:
            mav_handle_optical_flow(msg);
            break;

        default:
            printf("No handler for mavlink message %d\n", msg.msgid);
            fflush(stdout);
            break;
    }
}

// -- Mavlink handlers -- //
void AP::mav_handle_heartbeat(const mavlink_message_t& msg)
{
    //printf("BEEP %ld\n", millis());
    m_last_heartbeat = micros();
}

void AP::mav_handle_sys_status(const mavlink_message_t& msg)
{
    mavlink_sys_status_t status;
    mavlink_msg_sys_status_decode(&msg, &status);
    //printf(">>> AP_STATUS: Voltage: %d\n", status.voltage_battery);
}

void AP::mav_handle_autopilot_version(const mavlink_message_t& msg)
{
    mavlink_autopilot_version_t version;
    mavlink_msg_autopilot_version_decode(&msg, &version);
    printf("AUTOPILOT VERSION!\n");
}

void AP::mav_handle_statustext(const mavlink_message_t& msg)
{
    mavlink_statustext_t status_text;
    mavlink_msg_statustext_decode(&msg, &status_text);
    LOG.log(std::string(status_text.text), LOG_TYPE_AP);
    printf(">>> AP (%d): %s \n", status_text.severity, status_text.text);
}

void AP::mav_handle_distance_sensor(const mavlink_message_t& msg)
{
    mavlink_distance_sensor_t dist;
    mavlink_msg_distance_sensor_decode(&msg, &dist);
    printf("DIST: %d\n", dist.current_distance);
}

void AP::mav_handle_radio_status(const mavlink_message_t& msg)
{
    
}

void AP::mav_handle_gps_raw_int(const mavlink_message_t& msg)
{
    mavlink_gps_raw_int_t gps;
    mavlink_msg_gps_raw_int_decode(&msg, &gps);
    telemetry.gps_lat = gps.lat;
    telemetry.gps_lon = gps.lon;
    telemetry.gps_alt = gps.alt;
    telemetry.gps_vel = gps.vel;
    telemetry.gps_eph = gps.eph;
    telemetry.gps_epv = gps.epv;
    telemetry.gps_cog = gps.cog;
    telemetry.gps_fix_type = gps.fix_type;
    telemetry.gps_satellites_visible = gps.satellites_visible;
    telemetry.gps_h_acc = gps.h_acc;
    telemetry.gps_v_acc = gps.v_acc;
    telemetry.gps_vel_acc = gps.vel_acc;
}

void AP::mav_handle_scaled_imu(const mavlink_message_t& msg)
{
    mavlink_scaled_imu_t imu;
    mavlink_msg_scaled_imu_decode(&msg, &imu);
    telemetry.imu_xacc = imu.xacc;
    telemetry.imu_yacc = imu.yacc;
    telemetry.imu_zacc = imu.zacc;
    telemetry.imu_xgyro = imu.xgyro;
    telemetry.imu_ygyro = imu.ygyro;
    telemetry.imu_zgyro = imu.zgyro;
    telemetry.imu_xmag = imu.xmag;
    telemetry.imu_ymag = imu.ymag;
    telemetry.imu_zmag = imu.zmag;
    telemetry.imu_temperature = imu.temperature;
}

void AP::mav_handle_scaled_pressure(const mavlink_message_t& msg)
{
    mavlink_scaled_pressure_t pres;
    mavlink_msg_scaled_pressure_decode(&msg, &pres);
    telemetry.pres_abs = pres.press_abs;
    telemetry.temperature = (float) pres.temperature / 100.0;
}

void AP::mav_handle_attitude(const mavlink_message_t& msg)
{
    mavlink_attitude_t attitude;
    mavlink_msg_attitude_decode(&msg, &attitude);
    telemetry.roll = radians_to_degrees(attitude.roll);
    telemetry.pitch = radians_to_degrees(attitude.pitch);
    telemetry.yaw = radians_to_degrees(attitude.yaw);
    telemetry.rollspeed = radians_to_degrees(attitude.rollspeed);
    telemetry.pitchspeed = radians_to_degrees(attitude.pitchspeed);
    telemetry.yawspeed = radians_to_degrees(attitude.yawspeed);
}

void AP::mav_handle_local_position_ned(const mavlink_message_t& msg)
{
    printf("mav_handle_local_position_ned\n");
}

void AP::mav_handle_global_position_int(const mavlink_message_t& msg)
{
    mavlink_global_position_int_t pos;
    mavlink_msg_global_position_int_decode(&msg, &pos);
    telemetry.altitude = pos.alt;
    telemetry.relative_altitude = pos.relative_alt;
    telemetry.vx = pos.vx;
    telemetry.vy = pos.vy;
    telemetry.vz = pos.vz;
    telemetry.lat = pos.lat;
    telemetry.lon = pos.lon;
    telemetry.heading = pos.hdg;
}

void AP::mav_handle_rc_channels(const mavlink_message_t& msg)
{
    mavlink_rc_channels_t rc_channels;
    mavlink_msg_rc_channels_decode(&msg, &rc_channels);
    telemetry.rc_channel_1 = rc_channels.chan1_raw;
    telemetry.rc_rssi = rc_channels.rssi;
}

void AP::mav_handle_servo_output_raw(const mavlink_message_t& msg)
{
    mavlink_servo_output_raw_t servo_out;
    mavlink_msg_servo_output_raw_decode(&msg, &servo_out);
    telemetry.servo_out1 = servo_out.servo1_raw;
    telemetry.servo_out2 = servo_out.servo2_raw;
    telemetry.servo_out3 = servo_out.servo3_raw;
    telemetry.servo_out4 = servo_out.servo4_raw;
    telemetry.servo_out5 = servo_out.servo5_raw;
    telemetry.servo_out6 = servo_out.servo6_raw;
}

void AP::mav_handle_altitude(const mavlink_message_t& msg)
{
    printf("mav_handle_altitude\n");
}

void AP::mav_handle_battery_status(const mavlink_message_t& msg)
{
    mavlink_battery_status_t bat;
    mavlink_msg_battery_status_decode(&msg, &bat);
    telemetry.bat_voltage = (float) bat.voltages[0] / 1000.0;
    telemetry.bat_current = bat.current_battery;
    telemetry.bat_current_consumed = bat.current_consumed;
    telemetry.bat_battery_remaining = bat.battery_remaining;
}

void AP::mav_handle_vibration(const mavlink_message_t& msg)
{
    mavlink_vibration_t vibration;
    mavlink_msg_vibration_decode(&msg, &vibration);
    telemetry.vibration_x = vibration.vibration_x;
    telemetry.vibration_y = vibration.vibration_y;
    telemetry.vibration_z = vibration.vibration_z;
    telemetry.imu_clipping = vibration.clipping_0;
}

void AP::mav_handle_esc_status(const mavlink_message_t& msg)
{
    printf("mav_handle_esc_status\n");
}

void AP::mav_handle_optical_flow(const mavlink_message_t& msg)
{
    printf("mav_handle_optical_flow\n");
}



void AP::mav_handle_timesync(const mavlink_message_t& msg) {}

void AP::mav_handle_command_ack(const mavlink_message_t& msg)
{
    mavlink_command_ack_t ack;
    mavlink_msg_command_ack_decode(&msg, &ack);
    printf("COMMAND ACK: %d, res: %d, %d, %d\n", ack.command, ack.result, ack.progress, ack.result_param2);
}

void AP::mav_handle_param_value(const mavlink_message_t& msg)
{
    mavlink_param_value_t value;
    mavlink_msg_param_value_decode(&msg, &value);
    //printf(">>> AP_PARAM: %s: %f (type: %d)\n", value.param_id, value.param_value, value.param_type);
}



