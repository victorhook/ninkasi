#ifndef UTILS_H
#define UTILS_H


#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>


// Function to open and initialize the serial port
int serial_open(const char* portName, int baudRate);


template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    // Add an element to the queue
    void enqueue(T item);

    // Get the "front" element. Waits until the queue has data.
    T dequeue();

    // Get the "front" element without waiting. Returns false if the queue is empty.
    bool try_dequeue(T& item);

    // Check if the queue is empty
    bool empty() const;

    // Get the size of the queue
    size_t size() const;

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
};


template class ThreadSafeQueue<uint8_t>;

#endif /* UTILS_H */
