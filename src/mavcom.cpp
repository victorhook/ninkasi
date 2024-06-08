#include "mavcom.h"
#include "mavlink.h"
#include "utils.h"
#include "ap.h"

#include <netinet/in.h>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>


Mavcom::Mavcom(const char* serial_port, const int baudrate)
: m_baudrate(baudrate)
{
    strcpy(m_serial_port, serial_port);
}

Mavcom::~Mavcom()
{

}

bool Mavcom::init()
{
    m_is_running = true;

    std::thread th_serial_reader(&Mavcom::serial_reader, this);
    th_serial_reader.detach();

    std::thread th_tcp_proxy(&Mavcom::tcp_reader, this);
    th_tcp_proxy.detach();

    return false;
}

bool Mavcom::send()
{
    return false;
}

// -- Private -- //
void Mavcom::serial_reader()
{
    printf("Attempting to open serial connection to %s\n", m_serial_port);

    int fd = serial_open(m_serial_port, m_baudrate);

    if (fd < 0)
    {
        printf("Failed to open serial port at %s!", m_serial_port);
        return;
    }

    printf("Serial connection opened to port %s with baudrate %d\n", m_serial_port, m_baudrate);

    // Start writer thread
    std::thread(&Mavcom::serial_writer, this, fd).detach();

    bool running = true;
    mavlink_message_t msg;
    mavlink_status_t status;
    while (running)
    {
        uint8_t buf;
        int res = read(fd, &buf, 1);
        if (res < 0)
        {
            running = false;
        }
        else if (res > 0)
        {
            // First well parse incoming data as potential mavlink data
            if (mavlink_parse_char(MAVLINK_COMM_0, buf, &msg, &status))
            {
                ap.handle_mavlink_message(msg, status);
            }

            // After we've tried parsing the data, we'll put it to queue for potential proxying
            m_serial_queue_rx.enqueue(buf);
        }
    }

    printf("Serial connection %s closed", m_serial_port);
}


void Mavcom::tcp_reader()
{
    int sockfd = start_tcp_server();
    if (sockfd < 0)
    {
        return;
    }

    printf("TCP socket open on port %d\n", MAVPROXY_TCP_PORT);

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

        printf("New client fd: %d\n", client_fd);
        std::thread writer(&Mavcom::tcp_writer, this, client_fd);

        bool client_connected = true;
        while (client_connected)
        {
            uint8_t buf;
            int res = read(client_fd, &buf, 1);
            if (res == -1)
            {
                printf("TCP client %d closed\n", client_fd);
                client_connected = false;
            }
            else if (res > 0)
            {
                m_serial_queue_tx.enqueue(buf);
            }
        }

        writer.join();
        close(client_fd);
        //char client_ip[INET_ADDRSTRLEN];
        //inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
        //printf("Accepted connection from %s\n", client_ip);
        //close(new_socket);
    }

}

void handle_sigpipe(int sig) {
    std::cerr << "Caught SIGPIPE signal: " << sig << std::endl;
}

void Mavcom::tcp_writer(const int sockfd)
{
    signal(SIGPIPE, handle_sigpipe);

    bool running = true;
    while (running)
    {
        uint8_t data;
        if (m_serial_queue_rx.try_dequeue_for(data, 1000))
        {
            int bb = write(sockfd, &data, 1);
            if (bb < 0)
            {
                running = false;
            }
        }
        else
        {
            if (fcntl(sockfd, F_GETFL) == -1)
            {
                running = false;
            }
            continue;
        }
    }
}

void Mavcom::serial_writer(const int sockfd)
{
    bool running = true;
    while (running)
    {
        uint8_t data = m_serial_queue_tx.dequeue();
        if (write(sockfd, &data, 1) < 0)
        {
            close(sockfd);
            running = false;
        }
    }
}


int Mavcom::start_tcp_server()
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
