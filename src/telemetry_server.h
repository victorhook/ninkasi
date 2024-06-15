#ifndef TELEMETRY_SERVER_H
#define TELEMETRY_SERVER_H

#include "utils.h"
#include "telemetry.h"

class TelemetryServer : public TcpServer
{
    public:
        TelemetryServer(int port);

        /* Should be called at main loop fixed frequency */
        void update();

    protected:
        void handle_client(int client_socket) override;
        std::string name() const override;

    private:
        ThreadSafeQueue<Telemetry> m_telem_queue;
};

#endif /* TELEMETRY_SERVER_H */
