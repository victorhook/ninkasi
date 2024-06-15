#ifndef CAMERA_H
#define CAMERA_H

#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace libcamera;


class SimpleCamera
{
    public:
        SimpleCamera(const int width, const int height, const libcamera::PixelFormat pixelFormat = libcamera::formats::YUV420);
        ~SimpleCamera();
        void start();
        void stop();

    private:
        const int m_width;
        const int m_height;
        int m_stride;
        const libcamera::PixelFormat m_pixelFormat;

        std::shared_ptr<Camera> camera;
        std::unique_ptr<CameraConfiguration> config;
        std::unique_ptr<FrameBufferAllocator> allocator;
        Stream* stream;
        std::vector<std::unique_ptr<FrameBuffer>> buffers;
        std::unique_ptr<Request> req;
        CameraManager camera_manager;

        mutable std::mutex mutex_;
        std::condition_variable cond_var_;
        std::condition_variable full_cond_var_;
        bool request_completed;

        void camera_request_complete(Request *request);
        void camera_capture();
};


#endif /* CAMERA_H */
