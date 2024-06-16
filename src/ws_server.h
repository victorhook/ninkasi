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
        const int m_port;

        virtual std::string name() const = 0;

        void on_open(websocketpp::connection_hdl hdl);
        void on_close(websocketpp::connection_hdl hdl);
        void on_message(websocketpp::connection_hdl hdl, ws_server::message_ptr msg);
        bool send(const uint8_t* data, const size_t len);

    private:
        ws_server m_server;
        websocketpp::connection_hdl m_con;
        std::mutex m_mutex;
        bool m_connected;

        void run();

};


#endif /* WS_SERVER_H */
