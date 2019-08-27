#include "Connection.h"
#include "Pool.h"
#include "RingBuffer.h"
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

unordered_map<int, key_t *> src_memo;
const int PORT_NUMBER = 65536;
key_t *get_dst_block(int srcPort) {
    if (src_memo.find(srcPort) == src_memo.end()) {
        auto shmid = shmget(srcPort, PORT_NUMBER * sizeof(key_t), 0666 | IPC_CREAT);
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

Connection::Connection(uint32_t src_port, uint32_t dst_port) {
    src = src_port;
    dst = dst_port;
    readRB = rb_init(get_idx(src, dst));
    writeRB = rb_init(get_idx(dst, src));
}

Connection::~Connection() {
    rb_destroy(readRB);
    rb_destroy(writeRB);
}

int Connection::read(int len, char *output) {
    return rb_read(readRB, len, output);
}
int Connection::write(int len, char *input) {
    return rb_write(writeRB, len, input);
}