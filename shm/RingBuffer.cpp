#include "RingBuffer.h"
#include <atomic>
#include <cstring>
#include <cstdlib>
RingBuffer_t *rb_init(key_t key) {
	RingBuffer_t *rb = (RingBuffer_t *)malloc(sizeof(RingBuffer_t));
	if (!rb) return NULL;
	int shmid = shmget(key, SHM_SIZE, 0666|IPC_CREAT);
	shmid_ds info;
	shmctl(shmid, IPC_STAT, &info);
	char *address = (char *) shmat(shmid, (void *)0, 0);
	uint32_t *data = (uint32_t *)address;
	char *content = (char *)&data[2];
	if (info.shm_nattch == 0) {
		data[0] = data[1] = data[2] = 0;
	}
	rb->address = address;
	rb->content = content; 
	rb->shmid = shmid;
	return rb;
}

void rb_destroy(RingBuffer_t *rb) {
	shmdt(rb->address);
	shmid_ds info;
	shmctl(rb->shmid, IPC_STAT, &info);
	if (info.shm_nattch == 0) {
		shmctl(rb->shmid, IPC_RMID, NULL);
	}
}

int rb_read(RingBuffer_t *rb, int len, char *output) {
	uint32_t head, tail, new_head;
	int size;
retry:
	head = GET_HEAD(rb);
	tail = GET_OLDTAIL(rb);
	size = (tail >= head) ? (tail - head) : (tail + SHM_DATA_SIZE - head);
	if (size < len) {
		len = size;
	}
	if (len == 0) return 0;
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
	uint32_t head, tail, old_tail, new_tail;
	int size;
retry:
	head = GET_HEAD(rb);
	tail = GET_TAIL(rb);
	old_tail = GET_OLDTAIL(rb);
	size = (head > tail) ? (head - tail - 1) 
		: (head + SHM_DATA_SIZE - tail - 1);
	if (size < len) {
		len = size;
	}
	if (len == 0) return 0;
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
	while (!__sync_bool_compare_and_swap(&(GET_OLDTAIL(rb)), tail, new_tail));
	return len;
}
