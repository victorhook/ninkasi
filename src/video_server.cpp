#include "video_server.h"
#include <unistd.h>
#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>

#include "mapped_framebuffer.h"

#include <sys/mman.h> // Include for mmap


using namespace std;


VideoServer::VideoServer(int port) : TcpServer(port),
m_camera(640, 480)
{

}


void VideoServer::handle_client(int client_socket) { }

void VideoServer::go() {
    m_camera.start();
}

std::string VideoServer::name() const
{
    return "VideoServer";
}

void VideoServer::saveImage(FrameBuffer* buffer)
{

}
