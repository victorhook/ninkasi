#ifndef UTILS_H
#define UTILS_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>

#include "telemetry.h"

#include "ardupilotmega/mavlink.h"

// Function to open and initialize the serial port
int serial_open(const char* portName, int baudRate);

/* Function to get the current timestamp in microseconds */
uint64_t micros();

/* Function to get the current timestamp in milliseconds */
uint64_t millis();

/* Converts radians to degrees */
float radians_to_degrees(float radians);

class TcpServer {
public:
    TcpServer(int port);
    virtual ~TcpServer();
    void start();
    void stop();

protected:
    virtual void handle_client(int client_socket) = 0;
    virtual std::string name() const = 0;

private:
    int port;
    bool running;
    void run();
};


typedef enum
{
    LOG_TYPE_AP      = 1,
    LOG_TYPE_NINKASI = 2
} LogType;

typedef enum
{
    LOG_LEVEL_DEBUG   = 1,
    LOG_LEVEL_INFO    = 2,
    LOG_LEVEL_WARNING = 3,
    LOG_LEVEL_ERROR   = 4,
} LogLevel;

class LogBlock
{
    public:
        LogBlock(std::string msg, LogType type, LogLevel level);
        std::string msg;
        LogType type;
        LogLevel level;
};

template <typename T>
class ThreadSafeQueue {
public:
    explicit ThreadSafeQueue(size_t max_size) : max_size_(max_size) {}
    ~ThreadSafeQueue() = default;

    void enqueue(T item);
    T dequeue();
    bool try_dequeue(T& item);
    bool try_dequeue_for(T& item, uint64_t timeout_ms);

    bool empty() const;
    bool full() const;      // New method to check if the queue is full
    size_t size() const;

private:
    std::queue<T> queue_;
    size_t max_size_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
    std::condition_variable full_cond_var_;
};


template class ThreadSafeQueue<uint8_t>;
template class ThreadSafeQueue<mavlink_message_t>;
template class ThreadSafeQueue<Telemetry>;
template class ThreadSafeQueue<LogBlock>;


#endif /* UTILS_H */
