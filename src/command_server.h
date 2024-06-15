#ifndef COMMAND_SERVER_H
#define COMMAND_SERVER_H

#include "utils.h"
#include <string>


class CommandServer : public TcpServer
{
    public:
        CommandServer(int port);

    protected:
        void handle_client(int client_socket) override;
        std::string name() const override;

    private:
        void handle_command(const std::string& msg);
        void split_command(const std::string& input, std::string& command, std::vector<std::string>& params);
};


#endif /* COMMAND_SERVER_H */
