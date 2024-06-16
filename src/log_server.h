#ifndef LOG_SERVER_H
#define LOG_SERVER_H

#include "utils.h"
#include <string>
#include "ws_server.h"


class LogServer : public WsServer
{
    public:
        LogServer(int port);
        ~LogServer();
        void start() override;
        void log(std::string msg, LogType type = LOG_TYPE_NINKASI, LogLevel level = LOG_LEVEL_INFO);

    protected:
        std::string name() const override;

    private:
        void run();
        ThreadSafeQueue<LogBlock> m_log_queue;
};


#endif /* LOG_SERVER_H */
