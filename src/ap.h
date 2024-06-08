#ifndef AP_H
#define AP_H


#include "mavlink.h"


class AP
{
    public:
        AP();
        ~AP();
        void handle_mavlink_message(const mavlink_message_t& msg, const mavlink_status_t& status);

    private:
        uint32_t m_last_heartbeat;

        // Mavlink handlers
        void mav_handle_heartbeat(const mavlink_message_t& msg);
};

extern AP ap;


#endif /* AP_H */
