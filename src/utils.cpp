#include "utils.h"

#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>


// Function to open and initialize the serial port
int serial_open(const char* portName, int baudRate) {
    int serial_port = open(portName, O_RDWR);

    // Check for errors
    if (serial_port < 0) {
        std::cerr << "Error " << errno << " opening " << portName << ": " << strerror(errno) << std::endl;
        return -1;
    }

    // Configure serial port
    struct termios tty;
    memset(&tty, 0, sizeof tty);

    // Read in existing settings, and handle any error
    if (tcgetattr(serial_port, &tty) != 0) {
        std::cerr << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
        return -1;
    }

    // Set baud rate
    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);

    // 8N1 mode
    tty.c_cflag &= ~PARENB; // No parity bit
    tty.c_cflag &= ~CSTOPB; // Only one stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; // 8 bits per byte

    // Save settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        std::cerr << "Error " << errno << " from tcsetattr: " << strerror(errno) << std::endl;
        return -1;
    }

    return serial_port;
}


template <typename T>
void ThreadSafeQueue<T>::enqueue(T item) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(item));
    cond_var_.notify_one();
}

template <typename T>
T ThreadSafeQueue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] { return !queue_.empty(); });
    T item = std::move(queue_.front());
    queue_.pop();
    return item;
}

template <typename T>
bool ThreadSafeQueue<T>::try_dequeue(T& item) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return false;
    }
    item = std::move(queue_.front());
    queue_.pop();
    return true;
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template <typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

