#include "ap.h"
#include "utils.h"


AP::AP()
{

}

AP::~AP()
{

}

void AP::handle_mavlink_message(const mavlink_message_t& msg, const mavlink_status_t& status)
{
    //printf("Mavlink message: %d\n", msg.msgid);
    switch(msg.msgid)
    {
        case MAVLINK_MSG_ID_HEARTBEAT:
            mav_handle_heartbeat(msg);
            break;
        default:
            printf("No handler for mavlink message %d\n", msg.msgid);
            break;
    }
}

// -- Mavlink handlers -- //
void AP::mav_handle_heartbeat(const mavlink_message_t& msg)
{
    printf("BEEP %ld\n", millis());
    m_last_heartbeat = micros();
}



AP ap;
