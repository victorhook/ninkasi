#include "ws_server.h"
#include <iostream>


WsServer::WsServer(const int port) :
m_port(port)
{

}

WsServer::~WsServer() {}


void WsServer::start()
{
    m_server.init_asio();

    m_server.set_access_channels(websocketpp::log::alevel::all);
    m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    m_server.set_message_handler(std::bind(&WsServer::on_message, this, std::placeholders::_1, std::placeholders::_2));

    m_server.listen(m_port);
    m_server.start_accept();
    m_server.run();
}

void WsServer::on_message(websocketpp::connection_hdl hdl, ws_server::message_ptr msg) {
    std::cout << "Received: " << msg->get_payload() << std::endl;
    // Echo the message back
    m_server.send(hdl, msg->get_payload(), msg->get_opcode());
}