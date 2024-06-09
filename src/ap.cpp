#include "ap.h"
#include "utils.h"



AP::AP(const char* serial_port, const int baudrate) :
mavcom(serial_port, baudrate),
frame_number(0)
{

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

    mavcom.request_data_stream(MAVLINK_MSG_ID_SYS_STATUS, 1000);

    //mavcom.start_message_request_streams();
    return true;
}

void AP::update()
{
    if (frame_number % 25 == 0)
    {
        mavlink_message_t msg;
        mavlink_msg_heartbeat_pack(AP_SYSTEM_ID, AP_COMPONENT_ID, &msg, MAV_TYPE_ONBOARD_CONTROLLER, MAV_AUTOPILOT_INVALID, 0, 0, 0);
        mavcom.send_message(msg);
    }
}

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
        case MAVLINK_MSG_ID_STATUSTEXT:
            mav_handle_statustext(msg);
            break;
        case MAVLINK_MSG_ID_COMMAND_ACK:
            mav_handle_command_ack(msg);
            break;
        case MAVLINK_MSG_ID_PARAM_VALUE:
            mav_handle_param_value(msg);
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
    printf(">>> AP_STATUS: Voltage: %d\n", status.voltage_battery);
}

void AP::mav_handle_statustext(const mavlink_message_t& msg)
{
    mavlink_statustext_t status_text;
    mavlink_msg_statustext_decode(&msg, &status_text);
    printf(">>> AP (%d): %s \n", status_text.severity, status_text.text);
}


void AP::mav_handle_command_ack(const mavlink_message_t& msg)
{
    //mavlink_statustext_t status_text;
    //mavlink_msg_statustext_decode(&msg, &status_text);
    printf("COMMAND ACK!\n");
}

void AP::mav_handle_param_value(const mavlink_message_t& msg)
{
    mavlink_param_value_t value;
    mavlink_msg_param_value_decode(&msg, &value);
    printf(">>> AP_PARAM: %s: %f (type: %d)\n", value.param_id, value.param_value, value.param_type);
}



