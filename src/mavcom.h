#ifndef MAVCOM_H
#define MAVCOM_H

#include "ardupilotmega/mavlink.h"
#include "utils.h"

#define MAVPROXY_TCP_PORT 2223

#define AP_COMPONENT_ID MAV_COMP_ID_AUTOPILOT1
#define AP_SYSTEM_ID 1
#define NINKASI_SYSTEM_ID 100
#define NINKASI_COMPONENT_ID MAV_COMP_ID_ONBOARD_COMPUTER


class Mavcom
{
    public:
        Mavcom(const char* serial_port, const int baudrate);
        ~Mavcom();
        bool init();

        void request_data_stream(const uint32_t msgid, const uint32_t interval_ms);
        void send_command_int(uint16_t command, float param1 = 0, float param2 = 0, float param3 = 0, float param4 = 0, float param5 = 0, float param6 = 0, float param7 = 0);
        void send_message(mavlink_message_t& msg);

    private:
        bool m_is_running;
        char m_serial_port[128];
        int m_baudrate;
        int m_serial_fd;
        std::vector<int> tcp_clients;
        ThreadSafeQueue<mavlink_message_t> m_serial_mav_queue_tx;
        ThreadSafeQueue<mavlink_message_t> m_serial_mav_queue_rx;

        void tcp_reader();
        void tcp_writer(const int sockfd);
        void serial_reader();

        /* Thread that sends mavlink messages to serial connection to AP */
        void mavlink_serial_writer();

        int start_tcp_server();
};

#endif /* MAVCOM_H */
