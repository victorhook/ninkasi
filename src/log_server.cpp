#include "log_server.h"


LogServer::LogServer(int port) : WsServer(port),
m_log_queue(100)
{

}

LogBlock::LogBlock(std::string msg, LogType type, LogLevel level) :
msg(msg),
type(type),
level(level)
{

}


LogServer::~LogServer() {}

void LogServer::log(std::string msg, LogType type, LogLevel level)
{
    if (m_log_queue.full())
    {
        return;
    }
    m_log_queue.enqueue(LogBlock(msg, type, level));
}

void LogServer::start()
{
    WsServer::start();
    std::thread(&LogServer::run, this).detach();
}

void LogServer::run()
{
    bool running = true;
    bool take_from_queue = true;
    LogBlock log_block("", LOG_TYPE_AP, LOG_LEVEL_INFO);

    while (running)
    {
        if (take_from_queue)
        {
            log_block = m_log_queue.dequeue();
        }
        //printf("take_from_queue: %d\n", take_from_queue);

        std::ostringstream json;
        auto type = ((log_block.type == LOG_TYPE_AP) ? "ap" : "ninkasi");
        json << "{";
        json << "\"" << "type" << "\": \"" << type << "\", ";
        json << "\"" << "level" << "\": \"" << "LEVEL" << "\", ";
        json << "\"" << "msg" << "\": \"" << log_block.msg << "\"";
        json << "}";
        auto msg = json.str();

        //std::cout << "Sending log: " << msg.size() << ": " << msg << std::endl;
        take_from_queue = send(msg);

        if (!take_from_queue)
        {
            // If transmission failed, we'll wait a bit to prevent flooding
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}


std::string LogServer::name() const
{
    return "LogServer";
}

