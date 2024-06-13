#include "mavcom.h"
#include "utils.h"
#include "ap.h"

#include "mavlink/minimal/mavlink.h"
#include "mavlink/ardupilotmega/mavlink.h"

#include <thread>
#include <iostream>



Mavcom::Mavcom(const char* serial_port, const int baudrate) :
m_baudrate(baudrate),
m_serial_fd(-1),
m_serial_mav_queue_tx(100)
{
    strcpy(m_serial_port, serial_port);
    m_mavproxy = new Mavproxy((MavSerialCom*) this);
}

Mavcom::~Mavcom()
{

}

bool Mavcom::init()
{
    std::thread (&Mavcom::serial_reader, this).detach();
    std::thread (&Mavcom::mavlink_serial_writer, this).detach();
    //std::thread (&Mavcom::tcp_proxy_reader, this).detach();

    if (!m_mavproxy->init())
    {
        perror("Mavproxy init failed");
        return false;
    }

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

bool Mavcom::send_message(const mavlink_message_t& msg)
{
    if (m_serial_mav_queue_tx.full())
    {
        return false;
    }
    //printf("Queueing message %d (size: %ld)\n", msg.msgid, m_serial_mav_queue_tx.size());
    m_serial_mav_queue_tx.enqueue(msg);
    return true;
}

void Mavcom::add_listener(MavSerialListener* listener)
{
    m_mav_listeners.push_back(listener);
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
                for (const auto& listener : m_mav_listeners)
                {
                    listener->handle_message(msg);
                }
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
