#ifndef AP_H
#define AP_H

#include "mavcom.h"


class AP
{
    public:
        AP(const char* serial_port, const int baudrate);
        ~AP();
        bool init();

        /* Should be called from main loop at fixed frequency */
        void update();
        void handle_mavlink_message(const mavlink_message_t& msg, const mavlink_status_t& status);

        Mavcom mavcom;
        uint32_t frame_number;

    private:
        uint32_t m_last_heartbeat;

        // Mavlink handlers
        void mav_handle_heartbeat(const mavlink_message_t& msg);
        void mav_handle_sys_status(const mavlink_message_t& msg);
        void mav_handle_statustext(const mavlink_message_t& msg);
        void mav_handle_command_ack(const mavlink_message_t& msg);
        void mav_handle_param_value(const mavlink_message_t& msg);
};

extern AP ap;
extern uint32_t frame_number;


#endif /* AP_H */
