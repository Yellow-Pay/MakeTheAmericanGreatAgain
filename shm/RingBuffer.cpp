#include "RingBuffer.h"
#include "Pool.h"
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unordered_map>

using namespace std;

unordered_map<int, key_t *> src_memo;
const int PORT_NUMBER = 65536;
key_t *get_dst_block(int srcPort) {
    if (src_memo.find(srcPort) == src_memo.end()) {
        auto shmid =
            shmget(srcPort, PORT_NUMBER * sizeof(key_t), 0666 | IPC_CREAT);
        assert(shmid != -1);
        auto address = static_cast<key_t *>(shmat(shmid, (void *)0, 0));
        assert((char *)address + 1 != 0);
        if (address == 0)
            return NULL;
        src_memo[srcPort] = address;
        return address;
    } else {
        return src_memo[srcPort];
    }
}

key_t get_idx(int srcPort, int dstPort) {
#ifndef NDEBUG
    printf("RingBuffer get_idx: srcPort = 0x%lx, dstPort = 0x%lx\n", srcPort,
           dstPort);
#endif
    auto key_array = get_dst_block(srcPort);
    if (!key_array) {
        return -1;
    }
    key_t ret = key_array[dstPort];
#ifndef NDEBUG
    printf("get_array = 0x%lx\n", key_array[dstPort]);
#endif
    if (ret == 0) {
        ret = key_array[dstPort] = pool_get();
    }
    return ret;
}

void clear_idx(int srcPort, int dstPort) {
#ifndef NDEBUG
    printf("RingBuffer clear_idx: srcPort = 0x%lx, dstPort = 0x%lx\n", srcPort,
           dstPort);
#endif
    auto key_array = get_dst_block(srcPort);
    if (key_array == NULL)
        return;
    key_array[dstPort] = 0;
}

/*
Cache of RingBuffers in a process, to avoid extra shared memory accesses.
A process may access the same RingBuffer many times.
*/
unordered_map<int, RingBuffer_t *> rb_memo;
RingBuffer_t *rb_get(int k) {
    if (k == -1)
        return NULL;
    RingBuffer_t *rb = NULL;
    if (rb_memo.find(k) == rb_memo.end()) {
        rb = rb_init(k);
        if (!rb)
            return NULL;
        rb_memo[k] = rb;
    }
    return rb_memo[k];
}

RingBuffer_t *rb_init(int idx) {
    key_t key = idx << 16;
    RingBuffer_t *rb = (RingBuffer_t *)malloc(sizeof(RingBuffer_t));
    if (!rb)
        return NULL;
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    assert(shmid != -1);
    shmid_ds info;
    shmctl(shmid, IPC_STAT, &info);
    char *address = (char *)shmat(shmid, (void *)0, 0);
	assert((int)address != -1);
    assert(address != (char *)~0);
    uint32_t *data = (uint32_t *)address;
    char *content = (char *)&data[METADATA_SIZE];
    if (info.shm_nattch ==
        0) { // the shm segment is newly created, we need to initialize it
#ifndef NDEBUG
        cout << "rb_init reset shared memory: " << idx << endl;
#endif
        data[0] = data[1] = data[2] = 0;
    }
    rb->address = address;
    rb->content = content;
    rb->index = idx;
    rb->shmid = shmid;
#ifndef NDEBUG
    cout << "rb_init: " << idx << endl;
    cout << "rb: " << rb << endl;
    printf("init - rb->address = 0x%lx\n", address);
#endif
    return rb;
}

void rb_destroy(RingBuffer_t *rb) { pool_release(rb->index); }

int rb_read(RingBuffer_t *rb, int len, char *output) {
#ifndef NDEBUG
	printf("rb_read rb - 0x%lx\n", rb);
	printf("rb->address - 0x%lx\n", rb->address);
#endif
	uint32_t head, tail, new_head;
	int size;
retry:
    head = GET_HEAD(rb);
    tail = GET_OLDTAIL(rb);
    size = (tail >= head) ? (tail - head) : (tail + SHM_DATA_SIZE - head);
    if (size < len) {
        len = size;
    }
    if (len == 0)
        return 0;
    if (head + len < SHM_DATA_SIZE) {
        // one continuous segment
        memcpy(output, rb->content + head, len);
    } else {
        // read to tail, then read from begin
        int remain_length = SHM_DATA_SIZE - head;
        memcpy(output, rb->content + head, remain_length);
        memcpy(output + remain_length, rb->content, len - remain_length);
    }
    new_head = (head + len) % SHM_DATA_SIZE;
    if (!__sync_bool_compare_and_swap(&(GET_HEAD(rb)), head, new_head)) {
        goto retry;
    }
    return len;
}

int rb_write(RingBuffer_t *rb, int len, char *input) {
#ifndef NDEBUG
	printf("rb_write rb - 0x%lx\n", rb);
	printf("rb->address - 0x%lx\n", rb->address);
#endif
	uint32_t head, tail, old_tail, new_tail;
	int size;
retry:

    head = GET_HEAD(rb);
    tail = GET_TAIL(rb);
    old_tail = GET_OLDTAIL(rb);
    size =
        (head > tail) ? (head - tail - 1) : (head + SHM_DATA_SIZE - tail - 1);
    if (size < len) {
        len = size;
    }
    if (len == 0)
        return 0;
    new_tail = (tail + len) % SHM_DATA_SIZE;
    // atomic update tail - this thread can write
    if (!__sync_bool_compare_and_swap(&(GET_TAIL(rb)), tail, new_tail)) {
        goto retry;
    }
    if (tail + len < SHM_DATA_SIZE) {
        // one continuous write
        memcpy(rb->content + tail, input, len);
    } else {
        // write to end, then write from begin
        int remain_length = SHM_DATA_SIZE - tail;
        memcpy(rb->content + tail, input, remain_length);
        memcpy(rb->content, input + remain_length, len - remain_length);
    }
    // when GET_OLDTAIL == tail (previous write is finished)
    // update old_tail to new_tail to mark this write is finish
    while (!__sync_bool_compare_and_swap(&(GET_OLDTAIL(rb)), tail, new_tail))
        ;
    return len;
}
