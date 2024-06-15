#ifndef VIDEO_SERVER_H
#define VIDEO_SERVER_H

#include "utils.h"
#include "simple_camera.h"

using namespace libcamera;


class VideoServer : public TcpServer {

    public:
        VideoServer(int port);
        void go();

    protected:
        void handle_client(int client_socket) override;
        std::string name() const override;

    private:
        SimpleCamera m_camera;

        void saveImage(FrameBuffer* buffer);
};


#endif /* VIDEO_SERVER_H */
