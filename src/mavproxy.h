#ifndef MAVPROXY_H
#define MAVPROXY_H

#include "mavlink/ardupilotmega/mavlink.h"
#include "utils.h"


#define MAVPROXY_TCP_PORT 2223


class MavSerialListener
{
    public:
        virtual void handle_message(const mavlink_message_t& msg) = 0;
};

class MavSerialCom
{
    public:
        virtual bool send_message(const mavlink_message_t& msg) = 0;
        virtual void add_listener(MavSerialListener* listener) = 0;
};


class Mavproxy : public MavSerialListener
{
    public:
        Mavproxy(MavSerialCom* sender);
        ~Mavproxy();
        void handle_message(const mavlink_message_t& msg) override;
        bool init();

    private:
        MavSerialCom* m_sender;
        bool m_is_running;
        ThreadSafeQueue<mavlink_message_t> m_tcp_tx_queue;

        void reader_thread();
        void writer_thread(const int sockfd);
        int start_tcp_server();
};


#endif /* MAVPROXY_H */
