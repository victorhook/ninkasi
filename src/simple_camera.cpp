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


        libcamera::MappedFrameBuffer mappedBuffer(buffer, MappedFrameBuffer::MapFlag::Read);
        const std::vector<libcamera::Span<uint8_t>> mem = mappedBuffer.planes();

        /*
        void *mapped_mem = mmap(nullptr, frame_size, PROT_READ, MAP_SHARED, planeY.fd.get(), 0);

        if (mapped_mem == MAP_FAILED) {
            cerr << "Failed to mmap plane memory: " << strerror(errno) << endl;
            return;
        }

        printf("\n");
        printf("Total size: %ld, FD: %d, %d, %d, Length: %d, %d, %d, Offsets: %d, %d, %d, Invalid offsets: %d, %d, %d\n",
            frame_size,
            planeY.fd.get(), planeU.fd.get(), planeV.fd.get(),
            planeY.length, planeU.length, planeV.length,
            planeY.offset, planeU.offset, planeV.offset,
            planeY.kInvalidOffset, planeU.kInvalidOffset, planeV.kInvalidOffset
        );
        printf("\n");


        uint8_t* buf_y = &((uint8_t*) mapped_mem)[0];
        uint8_t* buf_u = &((uint8_t*) mapped_mem)[planeU.offset];
        uint8_t* buf_v = &((uint8_t*) mapped_mem)[planeV.offset];
        */
        printf("SEG??\n");

        //cv::Mat matY(m_height / 1, m_width / 1, CV_8U, buf_y, m_stride);
        //cv::Mat matU(m_height / 2, m_width / 2, CV_8U, buf_u, m_stride);
        //cv::Mat matV(m_height / 2, m_width / 2, CV_8U, buf_v, m_stride);
        cv::Mat matY(m_height / 1, m_width / 1, CV_8U, mem[0].data(), m_stride);
        cv::Mat matU(m_height / 2, m_width / 2, CV_8U, mem[1].data(), m_stride);
        cv::Mat matV(m_height / 2, m_width / 2, CV_8U, mem[2].data(), m_stride);

        printf("SEG2??\n");

        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        auto mem_left = pages * page_size;
        cout << "MEMORY LEFT: " << mem_left << endl;

        // Upsample U and V planes to match Y plane size
        cv::Mat matU_resized, matV_resized;
        cv::resize(matU, matU_resized, cv::Size(m_width, m_height), 0, 0, cv::INTER_LINEAR);
        cv::resize(matV, matV_resized, cv::Size(m_width, m_height), 0, 0, cv::INTER_LINEAR);

        printf("SEG3??\n");

        // Merge Y, U, and V planes into one YUV image
        std::vector<cv::Mat> yuv_planes = {matY, matU_resized, matV_resized};
        cv::Mat yuv_image;
        cv::merge(yuv_planes, yuv_image);

        printf("SEG24?\n");

        cv::Mat image;
        cv::cvtColor(yuv_image, image, cv::COLOR_YUV2RGB);

        //cv::Mat image(m_height, m_width, CV_8U, (uint8_t *)(mem[0].data()), m_stride);

        //cv::Mat bgr;
        //cv::cvtColor(image, bgr, cv::COLOR_YUV420p2BGR);

        auto filename = "image_super.jpg";
        cout << "Saving image to" << filename << endl;
        cv::imwrite(filename, image);

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