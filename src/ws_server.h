#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> ws_server;

class WsServer
{
    public:
        WsServer(const int port);
        ~WsServer();
        void start();

    protected:
        ws_server m_server;
        const int m_port;

        void on_message(websocketpp::connection_hdl hdl, ws_server::message_ptr msg);
};


#endif /* WS_SERVER_H */
