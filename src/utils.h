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

#include "ardupilotmega/mavlink.h"

// Function to open and initialize the serial port
int serial_open(const char* portName, int baudRate);

/* Function to get the current timestamp in microseconds */
uint64_t micros();

/* Function to get the current timestamp in milliseconds */
uint64_t millis();



class TcpServer {
public:
    TcpServer(int port);
    virtual ~TcpServer();
    void start();
    void stop();

protected:
    virtual void handle_client(int client_socket) = 0;
    virtual bool handle_in_thread() const = 0;

private:
    int port;
    bool running;
    std::thread server_thread;
    void run();
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

#endif /* UTILS_H */
