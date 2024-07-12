#include "simple_camera.h"

#include <opencv2/opencv.hpp>


#include "video_server.h"
#include <unistd.h>
#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>

#include "mapped_framebuffer.h"

#include <sys/mman.h> // Include for mmap
#include <unistd.h>


using namespace std;


SimpleCamera::SimpleCamera(const int width, const int height, const libcamera::PixelFormat pixelFormat) :
m_width(width),
m_height(height),
m_pixelFormat(pixelFormat)
{

}

SimpleCamera::~SimpleCamera()
{

}


void SimpleCamera::start() {
    camera_manager.start();
    auto cameras = camera_manager.cameras();

    if (cameras.empty())
    {
        perror("No camera found! Are you sure it is connected?");
        return;
    }

    camera = cameras[0];
    cout << "Using first found camera, with ID: " << camera->id() << endl;

    camera->acquire();

    config = camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
    if (!config) {
        cerr << "Error: Could not generate configuration." << endl;
        return;
    }

    config->at(0).size.width = m_width;
    config->at(0).size.height = m_height;
    config->at(0).pixelFormat = m_pixelFormat;
    config->validate();
    camera->configure(config.get());
    m_stride = config->at(0).stride;

    allocator = std::make_unique<libcamera::FrameBufferAllocator>(camera);
    stream = config->at(0).stream();

    if (allocator->allocate(stream) < 0)
    {
        std::cerr << "Failed to allocate buffers" << std::endl;
        return;
    }

    // Debug some info about configurations
    auto cp = camera->properties();
    auto cc = camera->controls();
    std::cout << "controls:\n";
    for (auto &c : cc)
    {
        std::cout << c.first->name() << ": " << c.second.toString() << " = " << c.second.def().toString() << std::endl;
    }
    std::cout << "properies:\n";
    for (auto &c : cp)
    {
        std::cout << c.first << ": " << c.second.toString() << std::endl;
    }


    req = camera->createRequest();
    req->addBuffer(stream, allocator->buffers(stream).at(0).get());
    req->reuse(Request::ReuseFlag::ReuseBuffers);

    camera->requestCompleted.connect(this, &SimpleCamera::camera_request_complete);

    //while (1)
    //{
    request_completed = false;

    camera->start();
    if (camera->queueRequest(req.get()) < 0) {
        cerr << "Failed to requeue request" << endl;
        return;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    full_cond_var_.wait(lock, [this] { return request_completed; });
    camera->stop();
    camera->release();
    //std::thread(&SimpleCamera::camera_capture, this).detach();
}

void SimpleCamera::camera_capture()
{
    uint32_t t0 = millis();
    int fps_counter = 0;
    while (1)
    {
        if ((millis() - t0) > 1000)
        {
            t0 = millis();
            cout << "FPS: " << fps_counter << endl;
            fps_counter = 0;
        }
        fps_counter++;
        std::unique_lock<std::mutex> lock(mutex_);
        full_cond_var_.wait(lock, [this] { return request_completed; });
        request_completed = false;

        req->reuse();
        req->addBuffer(stream, allocator->buffers(stream).at(0).get());

        int res = camera->queueRequest(req.get());
        if (res < 0) {
            cerr << "Failed to requeue request: " << res << endl;
            return;
        }

        //saveImage(allocator->buffers(stream).at(0).get());
        break;
    }
}


void SimpleCamera::camera_request_complete(Request *request)
{
    const Request::BufferMap &buffers = request->buffers();

    for (auto bufferPair : buffers)
    {
        const Stream *stream = bufferPair.first;
        FrameBuffer *buffer = bufferPair.second;
        const FrameMetadata &metadata = buffer->metadata();

        /* Print some information about the buffer which has completed. */
        std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence << " bytesused: ";

        // We're expecting the data to be in YUV
        unsigned int nplane = 0;
        for (const FrameMetadata::Plane &plane : metadata.planes())
        {
            std::cout << plane.bytesused;
            if (++nplane < metadata.planes().size())
                std::cout << "/";
        }
        cout << endl;

        auto planeY = buffer->planes().at(0);
        auto planeU = buffer->planes().at(1);
        auto planeV = buffer->planes().at(2);

        std::cout << " size " << m_width << "x" << m_height << " stride " << m_stride << " format " << m_pixelFormat.toString() << " sec "
                  << (double)clock() / CLOCKS_PER_SEC << std::endl;

        size_t frame_size = planeY.length + planeU.length + planeV.length;

        void* buf_y = mmap(nullptr, planeY.length, PROT_READ, MAP_SHARED, planeY.fd.get(), 0);
        void* buf_u = mmap(nullptr, planeU.length, PROT_READ, MAP_SHARED, planeU.fd.get(), 0);
        void* buf_v = mmap(nullptr, planeV.length, PROT_READ, MAP_SHARED, planeV.fd.get(), 0);


        if (buf_y == MAP_FAILED || buf_u == MAP_FAILED || buf_v == MAP_FAILED) {
            cerr << "Failed to mmap plane memory: " << strerror(errno) << endl;
            return;
        }

        if (munmap(buf_y, planeY.length) < 0 || munmap(buf_u, planeU.length) < 0 || munmap(buf_v, planeV.length) < 0) {
            cerr << "Failed to munmap plane memory: " << strerror(errno) << endl;
        }

        //libcamera::MappedFrameBuffer mappedBuffer(buffer, MappedFrameBuffer::MapFlag::Read);
        //const std::vector<libcamera::Span<uint8_t>> mem = mappedBuffer.planes();


        //cv::Mat bgr;
        //cv::cvtColor(image, bgr, cv::COLOR_YUV420p2BGR);

        //auto filename = "image_super.jpg";
        //cout << "Saving image to" << filename << endl;
        //cv::imwrite(filename, image);

        //if (munmap(mapped_mem, planeY.length) < 0) {
        //    cerr << "Failed to munmap plane memory: " << strerror(errno) << endl;
        //}
    }

    std::lock_guard<std::mutex> _lock(mutex);
    request_completed = true;
    full_cond_var_.notify_one();
}

/*
void SimpleCamera::saveImage(FrameBuffer* buffer)
{

}
*/