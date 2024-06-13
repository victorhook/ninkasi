#ifndef MAVCOM_H
#define MAVCOM_H

#include "ardupilotmega/mavlink.h"
#include "utils.h"
#include "mavproxy.h"

#define AP_COMPONENT_ID MAV_COMP_ID_AUTOPILOT1
#define AP_SYSTEM_ID 1
#define NINKASI_SYSTEM_ID 100
#define NINKASI_COMPONENT_ID MAV_COMP_ID_ONBOARD_COMPUTER


class Mavcom : public MavSerialCom, public std::enable_shared_from_this<MavSerialCom>
{
    public:
        Mavcom(const char* serial_port, const int baudrate);
        ~Mavcom();
        bool init();

        void request_data_stream(const uint32_t msgid, const uint32_t interval_ms);
        void send_command_int(uint16_t command, float param1 = 0, float param2 = 0, float param3 = 0, float param4 = 0, float param5 = 0, float param6 = 0, float param7 = 0);

        bool send_message(const mavlink_message_t& msg) override;
        void add_listener(MavSerialListener* listener) override;

    private:
        bool m_is_running;
        char m_serial_port[128];
        int m_baudrate;
        int m_serial_fd;
        std::vector<MavSerialListener*> m_mav_listeners;

        Mavproxy* m_mavproxy;

        ThreadSafeQueue<mavlink_message_t> m_serial_mav_queue_tx;

        void serial_reader();

        /* Thread that sends mavlink messages to serial connection to AP */
        void mavlink_serial_writer();
};

#endif /* MAVCOM_H */
