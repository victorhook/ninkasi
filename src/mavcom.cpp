#include "mavcom.h"
#include "mavlink.h"
#include <netinet/in.h>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

#include "utils.h"


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
    printf("Serial reader started\n");

    int fd = serial_open(m_serial_port, m_baudrate);

    if (fd < 0)
    {
        printf("Failed to open serial port at %s!", m_serial_port);
        return;
    }

    printf("Serial connection opened to port %s with baudrate %d\n", m_serial_port, m_baudrate);

    std::thread th_serial_writer(&Mavcom::serial_writer, this, fd);
    th_serial_writer.detach();

    while (1)
    {
        uint8_t buf;
        int res = read(fd, &buf, 1);
        if (res < 0)
        {
            printf("Serial connection problems... Exiting!");
            return;
        }
        else if (res > 0)
        {
            m_serial_queue_rx.enqueue(buf);
            //printf("%02x ", buf);
            //fflush(stdout);
        }
    }

}

void Mavcom::tcp_reader()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == 0)
    {
        perror("Failed to open TCP proxy server!");
        return;
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setting socket options failed!");
        return;
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MAVPROXY_TCP_PORT);

    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(sock, 3) < 0) {
        perror("listen");
        return;
    }

    printf("TCP socket open on port %d\n", MAVPROXY_TCP_PORT);

    while (m_is_running)
    {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_fd = accept(sock, (struct sockaddr *)&client_address, &client_address_len);
        if (client_fd < 0)
        {
            perror("accept failed");
            continue;
        }

        printf("New client fd: %d\n", client_fd);
        std::thread(&Mavcom::tcp_writer, this, client_fd).detach();

        bool client_connected = true;
        while (client_connected)
        {
            uint8_t buf;
            int res = read(client_fd, &buf, 1);
            if (res == -1)
            {
                printf("TCP client %d closed\n", client_fd);
                close(client_fd);
                client_connected = false;
            }
            else if (res > 0)
            {
                m_serial_queue_tx.enqueue(buf);
                //mavlink_message_t msg;
                //mavlink_status_t status;
                //if (mavlink_parse_char(0, buf, &msg, &status))
                //{
                //    printf("MESSAGE ID: %d", msg.msgid);
                //}
                //printf("TCP [%d] data: %02x\n", sockfd, buf);
            }
        }

        //tcp_clients.push_back(client_fd);
        //std::thread(handler, client_fd).detach();

        //char client_ip[INET_ADDRSTRLEN];
        //inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
        //printf("Accepted connection from %s\n", client_ip);
        //close(new_socket);
    }

}


void Mavcom::tcp_writer(const int sockfd)
{
    bool running = true;
    while (running)
    {
        uint8_t data = m_serial_queue_rx.dequeue();
        if (write(sockfd, &data, 1) < 0)
        {
            close(sockfd);
            running = false;
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
