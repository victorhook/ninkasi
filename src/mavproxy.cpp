#include "mavproxy.h"

#include <thread>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>

void handle_sigpipe(int sig);



Mavproxy::Mavproxy(MavSerialCom* sender) :
m_sender(sender),
m_is_running(false),
m_tcp_tx_queue(100)
{

}

Mavproxy::~Mavproxy()
{

}

bool Mavproxy::init()
{
    m_sender->add_listener((MavSerialListener*) this);
    std::thread(&Mavproxy::reader_thread, this).detach();
    return true;
}

void Mavproxy::handle_message(const mavlink_message_t& msg)
{
    if (!m_tcp_tx_queue.full())
    {
        m_tcp_tx_queue.enqueue(msg);
    }
}


void Mavproxy::reader_thread()
{
    int sockfd = start_tcp_server();
    if (sockfd < 0)
    {
        return;
    }

    printf("[MAVPROXY] TCP socket open on port %d\n", MAVPROXY_TCP_PORT);
    m_is_running = true;

    while (m_is_running)
    {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_fd = accept(sockfd, (struct sockaddr *)&client_address, &client_address_len);
        if (client_fd < 0)
        {
            perror("accept failed\n");
            continue;
        }

        printf("[MAVPROXY] New client fd: %d\n", client_fd);
        std::thread writer(&Mavproxy::writer_thread, this, client_fd);

        bool client_connected = true;
        mavlink_message_t mav_msg;
        mavlink_status_t mav_status;
        while (client_connected)
        {
            uint8_t buf;
            int res = read(client_fd, &buf, 1);
            if (res < 1)
            {
                printf("[MAVPROXY] TCP client %d closed\n", client_fd);
                client_connected = false;
            }
            else if (res > 0)
            {
                // We've received data that should be tunneled to AP. We'll try to parse it into mavlink data
                if (mavlink_parse_char(MAVLINK_COMM_1, buf, &mav_msg, &mav_status))
                {
                    m_sender->send_message(mav_msg);
                }
            }
        }

        printf("[MAVPROXY] Closing client connection");
        writer.join();
        close(client_fd);
    }

}


void Mavproxy::writer_thread(const int sockfd)
{
    signal(SIGPIPE, handle_sigpipe);

    static uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    bool running = true;
    while (running)
    {
        mavlink_message_t msg;
        if (m_tcp_tx_queue.try_dequeue_for(msg, 1000))
        {
            uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
            if (write(sockfd, buf, len) < 0)
            {
                running = false;
            }
        }
        else
        {
            // Check if socket is still open and stop running if it's not
            if (fcntl(sockfd, F_GETFL) == -1)
            {
                running = false;
            }
            continue;
        }
    }
}


int Mavproxy::start_tcp_server()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == 0)
    {
        perror("Failed to open TCP proxy server!");
        return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setting socket options failed!");
        return -1;
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MAVPROXY_TCP_PORT);

    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    if (listen(sockfd, 3) < 0) {
        perror("listen");
        return -1;
    }

    return sockfd;
}



void handle_sigpipe(int sig) {
    std::cerr << "Caught SIGPIPE signal: " << sig << std::endl;
}
