#include "telemetry_server.h"
#include "ap.h"
#include <thread>


TelemetryServer::TelemetryServer(int port) : WsServer(port),
m_telem_queue(5)
{

}

void TelemetryServer::update()
{
    if ((frame_number % 5) == 0 && (!m_telem_queue.full()))
    {
        m_telem_queue.enqueue(ap.telemetry);
    }
}

void TelemetryServer::go()
{
    std::thread(&TelemetryServer::handle_client, this, 1).detach();
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
        //printf("take_from_queue: %d\n", take_from_queue);
        take_from_queue = send((uint8_t*) &telemetry, sizeof(Telemetry));
        if (!take_from_queue)
        {
            // If transmission failed, we'll wait a bit to prevent flooding
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}

std::string TelemetryServer::name() const
{
    return "TelemetryServer";
}
