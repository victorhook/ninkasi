#ifndef LOG_SERVER_H
#define LOG_SERVER_H

#include "utils.h"
#include <string>

typedef enum
{
    LOG_TYPE_AP      = 1,
    LOG_TYPE_NINKASI = 2
} LogType;

typedef struct
{

} LogBlock;

class LogBlock
{
    public:
        LogBlock();
};

class LogServer : public TcpServer
{
    public:
        LogServer(int port);
        void addLog(const LogBlock log);

    protected:
        void handle_client(int client_socket) override;
        bool handle_in_thread() const override;
};


#endif /* LOG_SERVER_H */
