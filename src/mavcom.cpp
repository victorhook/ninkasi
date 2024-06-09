#include "mavcom.h"
#include "utils.h"
#include "ap.h"

#include "mavlink/minimal/mavlink.h"
#include "mavlink/ardupilotmega/mavlink.h"

#include <netinet/in.h>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>


Mavcom::Mavcom(const char* serial_port, const int baudrate) :
m_baudrate(baudrate),
m_serial_fd(-1),
m_serial_mav_queue_tx(100),
m_serial_mav_queue_rx(100)
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

    //std::thread th_tcp_proxy(&Mavcom::tcp_reader, this);
    //th_tcp_proxy.detach();

    std::thread(&Mavcom::mavlink_serial_writer, this).detach();

    return true;
}


void Mavcom::send_command_int(uint16_t command, float param1, float param2, float param3, float param4, float param5, float param6, float param7)
{
    mavlink_message_t msg;
    mavlink_msg_command_int_pack(
        NINKASI_SYSTEM_ID, NINKASI_COMPONENT_ID, &msg, AP_SYSTEM_ID, AP_COMPONENT_ID, 0,
        command,
        0, 0, // Not used
        param1,
        param2,
        param3,
        param4,
        param5,
        param6,
        param7
    );
    send_message(msg);
}

void Mavcom::request_data_stream(const uint32_t msgid, const uint32_t interval_ms)
{
    mavlink_message_t msg;
    mavlink_msg_command_int_pack(
        NINKASI_SYSTEM_ID, NINKASI_COMPONENT_ID, &msg, AP_SYSTEM_ID, AP_COMPONENT_ID, MAV_FRAME_MISSION,
        MAV_CMD_SET_MESSAGE_INTERVAL,
        0, 0, // Not used
        msgid,
        interval_ms * 1000, // Interval is expected in us
        0,
        0,
        0,
        0,
        0
    );
    send_message(msg);
}


void Mavcom::send_message(mavlink_message_t& msg)
{
    //printf("Queueing message %d (size: %ld)\n", msg.msgid, m_serial_mav_queue_tx.size());
    m_serial_mav_queue_tx.enqueue(msg);
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
    m_serial_fd = fd;

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
                //printf("NEW MSG: %d (err: %d)\n", msg.msgid, status.parse_error);
                ap.handle_mavlink_message(msg, status);

                // After we've tried parsing the data, we'll put it to queue for potential proxying
                //if (!m_serial_mav_queue_rx.full())
                //{
                //    m_serial_mav_queue_rx.enqueue(msg);
                //}
            }
        }
    }

    printf("Serial connection %s closed", m_serial_port);
}


void Mavcom::mavlink_serial_writer()
{
    static uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    static mavlink_message_t msg;
    bool take_from_queue = true;

    while (1)
    {
        if (take_from_queue)
        {
            msg = m_serial_mav_queue_tx.dequeue();
        }

        //printf("Sending message: %d, taking from queue: %d", msg.msgid, take_from_queue);

        uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
        ssize_t bytes_written = write(m_serial_fd, buf, len);

        // We'll only take next message from queue if we successfully sent the message
        take_from_queue = bytes_written == len;
        if (!take_from_queue)
        {
            // If the transmission failed, we'll wait a bit to avoid spamming the connection
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        //printf(" (%ld/%d sent)\n", bytes_written, len);
    }
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
        mavlink_message_t mav_msg;
        mavlink_status_t mav_status;
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
                // We've received data that should be tunneled to AP. We'll try to parse it into mavlink data
                if (mavlink_parse_char(MAVLINK_COMM_1, res, &mav_msg, &mav_status))
                {
                    m_serial_mav_queue_tx.enqueue(mav_msg);
                }

                //m_serial_queue_tx.enqueue(buf);
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

    static uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    bool running = true;
    while (running)
    {
        //uint8_t data;
        mavlink_message_t msg;
        if (m_serial_mav_queue_rx.try_dequeue_for(msg, 1000))
        {
            uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
            int bb = write(sockfd, buf, len);
            //int bb = write(sockfd, &data, 1);
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
