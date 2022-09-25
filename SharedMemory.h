#pragma once
#include <array>
#include <atomic>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

namespace MM {
template <class T, size_t size>
class CircleBuffer {
    std::atomic<int> read_head = 0;
    std::atomic<int> write_head = 0;
    std::atomic<bool> is_full = { false };

public:
    T buffer[size];
    CircleBuffer() = default;
    void write(T input) {
        if (is_full)
            return;

        buffer[write_head] = input;
        write_head = (write_head + 1) % size;
        // FIXME: could be slow hee
        if (read_head == write_head) {
            is_full = true;
        }
    }
    template <size_t consumer>
    T read() {
        if (is_empty<consumer>())
            return T();

        T element = buffer[read_head];
        read_head = (read_head + 1) % size;
        is_full = false;
        return element;
    }
    template <size_t consumer>
    bool is_empty() {
        return read_head == write_head && !is_full;
    }
} __attribute__((__packed__));

struct IpcHandler {
    std::atomic<int> block_size = 0;
    std::atomic<int> sample_rate = 0;
    std::atomic<int64_t> current_id = 0;
    MM::CircleBuffer<float, 8192 * 2> buffer;
};

}

#define IPC_FILE_NAME "MM Server"
#define IPC_TYPE MM::IpcHandler
#define BLOCK_SIZE sizeof(IPC_TYPE)