#include "utils.h"

#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <vector>
#include <memory>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>

auto start_time = std::chrono::steady_clock::now();

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

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= 0;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    // Save settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        std::cerr << "Error " << errno << " from tcsetattr: " << strerror(errno) << std::endl;
        return -1;
    }

    return serial_port;
}

uint64_t micros() {
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}

// Function to get the current timestamp in milliseconds
uint64_t millis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}




TcpServer::TcpServer(int port) : port(port), running(false) {}

TcpServer::~TcpServer() {
    stop();
}

void TcpServer::start()
{
    std::thread(&TcpServer::run, this).detach();
}

void TcpServer::stop() {
    running = false;
}

void TcpServer::run() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket\n");
        return;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setting socket options failed!");
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind socket");
        close(server_socket);
        return;
    }

    if (listen(server_socket, 10) == -1) {
        perror("Failed to listen on socket");
        close(server_socket);
        return;
    }

    running = true;
    std::cout << name() << ": server started on port " << port << std::endl;

    while (running) {
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Failed to accept client connection");
            continue;
        }

        std::cout << name() << ": New client connected" << std::endl;

        handle_client(client_socket);
    }

    close(server_socket);
}


template <typename T>
void ThreadSafeQueue<T>::enqueue(T item) {
    std::unique_lock<std::mutex> lock(mutex_);
    full_cond_var_.wait(lock, [this] { return queue_.size() < max_size_; });
    queue_.push(std::move(item));
    cond_var_.notify_one();
}

template <typename T>
T ThreadSafeQueue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] { return !queue_.empty(); });
    T item = std::move(queue_.front());
    queue_.pop();
    full_cond_var_.notify_one();
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
    full_cond_var_.notify_one();
    return true;
}

template <typename T>
bool ThreadSafeQueue<T>::try_dequeue_for(T& item, uint64_t timeout_ms) {
    using namespace std::chrono;
    auto timeout_duration = milliseconds(timeout_ms);
    std::unique_lock<std::mutex> lock(mutex_);

    if (!cond_var_.wait_for(lock, timeout_duration, [this] { return !queue_.empty(); })) {
        return false; // Timeout occurred
    }

    item = std::move(queue_.front());
    queue_.pop();
    full_cond_var_.notify_one();
    return true; // Successfully dequeued item
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template <typename T>
bool ThreadSafeQueue<T>::full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size() >= max_size_;
}

template <typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}
