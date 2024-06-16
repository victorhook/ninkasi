#ifndef TELEMETRY_SERVER_H
#define TELEMETRY_SERVER_H

#include "utils.h"
#include "telemetry.h"
#include "ws_server.h"

class TelemetryServer : public WsServer
{
    public:
        TelemetryServer(int port);

        /* Should be called at main loop fixed frequency */
        void update();

        void go();

    protected:
        std::string name() const override;

    private:
        void handle_client(int client_socket);
        ThreadSafeQueue<Telemetry> m_telem_queue;
};

#endif /* TELEMETRY_SERVER_H */
