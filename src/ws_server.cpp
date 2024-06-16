#include "ws_server.h"
#include <iostream>
#include <boost/asio.hpp>


WsServer::WsServer(const int port) :
m_port(port)
{

}

WsServer::~WsServer() {}


void WsServer::start()
{
    std::thread(&WsServer::run, this).detach();
}

void WsServer::run()
{
    m_server.init_asio();

    m_server.clear_access_channels(websocketpp::log::alevel::all); // Disable all access logging
    m_server.clear_error_channels(websocketpp::log::elevel::all); // Disable all error logging
    m_server.set_reuse_addr(true);

    // Add event handlers
    m_server.set_open_handler(std::bind(&WsServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(std::bind(&WsServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(std::bind(&WsServer::on_message, this, std::placeholders::_1, std::placeholders::_2));

    try {
        m_server.listen(m_port);
        m_server.start_accept();
        m_server.run();
        std::cout << "Server ended now...." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "HAH!![[[[[ Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "HAH!![[[[[ Unknown exception" << std::endl;
    }
}

void WsServer::on_message(websocketpp::connection_hdl hdl, ws_server::message_ptr msg) {
    std::cout << "Received: " << msg->get_payload() << std::endl;
    // Echo the message back
    //m_server.send(hdl, msg->get_payload(), msg->get_opcode());
}

void WsServer::on_open(websocketpp::connection_hdl hdl) {
    std::cout << "Client connected" << std::endl;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connected = true;
        m_con = hdl;
    }
    //on_connect();
}

void WsServer::on_close(websocketpp::connection_hdl hdl) {
    std::cout << "Client disconnected" << std::endl;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connected = false;
    }
    //on_disconnect();
}

bool WsServer::send(const uint8_t* data, const size_t len)
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_connected)
        {
            return false;
        }

        //std::cout << "Sending data: " << len << " bytes" << std::endl;

        auto con = m_con.lock();
        if (!con)
        {
            return false;
        }

        auto hdl = m_server.get_con_from_hdl(con);
        if (hdl && hdl->get_state() == websocketpp::session::state::open)
        {
            m_server.send(con, (const void*) data, len, websocketpp::frame::opcode::binary);
            return true;
        }

        std::cout << "Returning 1" << std::endl;
        return false;
    } catch (...) {
        std::cerr << "HAH!![[[[[ Unknown exception" << std::endl;
    }

    std::cout << "Returning 2" << std::endl;

    return false;
}

bool WsServer::send(std::string& msg)
{
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_connected)
        {
            return false;
        }

        std::cout << "Sending data: " << msg.size() << " characters" << std::endl;

        auto con = m_con.lock();
        if (!con)
        {
            return false;
        }

        auto hdl = m_server.get_con_from_hdl(con);
        if (hdl && hdl->get_state() == websocketpp::session::state::open)
        {
            m_server.send(con, msg, websocketpp::frame::opcode::TEXT);
            return true;
        }

        std::cout << "Returning 1" << std::endl;
        return false;
    } catch (...) {
        std::cerr << "HAH!![[[[[ Unknown exception" << std::endl;
    }

    std::cout << "Returning 2" << std::endl;

    return false;
}