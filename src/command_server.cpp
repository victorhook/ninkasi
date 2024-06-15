#include "command_server.h"
#include "ap.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

CommandServer::CommandServer(int port) : TcpServer(port)
{

}

void CommandServer::handle_client(int client_socket)
{
    bool running = true;
    char buf[1024];
    int index = 0;
    while (running)
    {
        char c;
        int bytes_read = read(client_socket, &c, 1);

        if (bytes_read > 0)
        {
            if (c == '\n')
            {
                buf[index + 1] = 0;
                handle_command(std::string(buf));
                index = 0;
            }
            else
            {
                buf[index++] = c;
            }
        }
        else if (bytes_read < 0)
        {
            running = false;
        }
    }

    printf("Connection to socket %d closed\n", client_socket);
}

std::string CommandServer::name() const
{
    return "CommandServer";
}


void CommandServer::handle_command(const std::string& cmd)
{
    std::string command;
    std::vector<std::string> params;
    split_command(cmd, command, params);

    printf("COMMAND RECEIVED: %s\n", cmd.c_str());
    printf("  COMMAND IS: %s\n", command.c_str());
    printf("  Params are: ");
    for (size_t i = 0; i < params.size(); i++)
    {
        printf("%s, ", params[i].c_str());
    }
    printf("\n");


    if (command.compare("ap_reboot") == 0)
    {
        printf("Rebooting AP\n");
        ap.mavcom.send_command_int(MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN, 1);
    }

}

void CommandServer::split_command(const std::string& input, std::string& command, std::vector<std::string>& params) {
    std::istringstream iss(input);
    iss >> command; // Read the first word as the command

    std::string param;
    while (iss >> param) { // Read the remaining words as parameters
        params.push_back(param);
    }
}