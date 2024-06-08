#ifndef MAVCOM_H
#define MAVCOM_H

#include "utils.h"

#define MAVPROXY_TCP_PORT 2223


class Mavcom
{
    public:
        Mavcom(const char* serial_port, const int baudrate);
        ~Mavcom();
        bool init();
        bool send();

    private:
        bool m_is_running;
        char m_serial_port[128];
        int m_baudrate;
        std::vector<int> tcp_clients;
        ThreadSafeQueue<uint8_t> m_serial_queue_rx;
        ThreadSafeQueue<uint8_t> m_serial_queue_tx;

        void tcp_reader();
        void tcp_writer(const int sockfd);
        void serial_reader();
        void serial_writer(const int sockfd);
};

#endif /* MAVCOM_H */
