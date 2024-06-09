#include "telemetry_server.h"
#include "ap.h"


TelemetryServer::TelemetryServer(int port) : TcpServer(port),
m_telem_queue(5)
{

}

void TelemetryServer::update()
{
    m_telem_queue.enqueue(ap.telemetry);
}

void TelemetryServer::handle_client(int client_socket)
{
    bool running = true;
    bool take_from_queue = true;
    Telemetry telemetry;

    while (running)
    {
        if (take_from_queue)
        {
            telemetry = m_telem_queue.dequeue();
        }

        ssize_t bytes_sent = write(client_socket, (uint8_t*) &telemetry, sizeof(Telemetry));
        take_from_queue = bytes_sent == sizeof(Telemetry);
    }
}

bool TelemetryServer::handle_in_thread() const
{
    return false;
}

