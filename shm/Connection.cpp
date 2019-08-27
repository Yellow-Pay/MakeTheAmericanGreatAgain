#include "Pool.h"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <sys/shm.h>
#include <unordered_map>

using namespace std;

#define SHM_SIZE 32
unordered_map<int, key_t *> src_memo;
key_t *get_dst_block(int srcPort) {
    if (src_memo.find(srcPort) == src_memo.end()) {
        auto shmid = shmget(srcPort, SHM_SIZE, 0666 | IPC_CREAT);
        auto address = (key_t *)shmat(shmid, (void *)0, 0);
        src_memo[srcPort] = address;
        return address;
    } else {
        return src_memo[srcPort];
    }
}
key_t get_idx(int srcPort, int dstPort) {
    auto key_array = get_dst_block(srcPort);
    key_t ret = key_array[dstPort];
    if (ret == 0) {
        ret = key_array[dstPort] = pool_get();
    }
    return ret;
}
#define SHM_DATA_SIZE (SHM_SIZE - 2 * sizeof(uint32_t))
#define GET_HEAD ((uint32_t *)address)[0]
#define GET_TAIL ((uint32_t *)address)[1]

struct RingBuffer {
    int shmid;
    int index;
    char *address;
    char *content;
    RingBuffer(int idx) {
        index = idx;
        key_t key = idx << 16;
        shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
        shmid_ds info;
        shmctl(shmid, IPC_STAT, &info);
        address = (char *)shmat(shmid, (void *)0, 0);
        uint32_t *data = (uint32_t *)address;
        content = (char *)&data[2];
        if (info.shm_nattch == 0) {
            data[0] = 0;
            data[1] = 0;
        }
    }
    ~RingBuffer() { pool_release(index); }

    int read(int len, char *output) {
        uint32_t head, tail, new_head;
        int size;
    retry:
        head = GET_HEAD;
        tail = GET_TAIL;
        size = (tail >= head) ? (tail - head) : (tail + SHM_DATA_SIZE - head);
        if (size < len) {
            len = size;
        }
        if (len == 0)
            return 0;
        if (head + len < SHM_DATA_SIZE) {
            memcpy(output, content + head, len);
        } else {
            int remain_length = SHM_DATA_SIZE - head;
            memcpy(output, content + head, remain_length);
            memcpy(output + remain_length, content, len - remain_length);
        }
        new_head = (head + len) % SHM_DATA_SIZE;
        if (!__sync_bool_compare_and_swap(&(GET_HEAD), head, new_head)) {
            goto retry;
        }
        return len;
    }

    // Write as many as possible when the ringbuffer is full
    int write(int len, const char *input) {
        uint32_t head, tail, new_tail;
        int size;
    retry:
        head = GET_HEAD;
        tail = GET_TAIL;
        size = (head > tail) ? (head - tail - 1)
                             : (head + SHM_DATA_SIZE - tail - 1);
        if (size < len) {
            len = size;
        }
        if (len == 0)
            return 0;
        new_tail = (tail + len) % SHM_DATA_SIZE;
        if (!__sync_bool_compare_and_swap(&(GET_TAIL), tail, new_tail)) {
            goto retry;
        }
        if (tail + len < SHM_DATA_SIZE) {
            memcpy(content + tail, input, len);
        } else {
            int remain_length = SHM_DATA_SIZE - tail;
            memcpy(content + tail, input, remain_length);
            memcpy(content, input + remain_length, len - remain_length);
        }
        return len;
    }
};

struct Connection {
    uint32_t src;
    uint32_t dst;
    RingBuffer readRB, writeRB;
    Connection(uint32_t src_port, uint32_t dst_port)
        : src(src_port), dst(dst_port), readRB(get_idx(src, dst)),
          writeRB(get_idx(dst, src)) {}
    ~Connection() {}
    int read(int len, char *output) { return readRB.read(len, output); }
    int write(int len, const char *input) { return writeRB.write(len, input); }
};