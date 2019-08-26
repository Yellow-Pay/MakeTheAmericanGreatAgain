#ifndef __SHM_CONNECTION_H
#define __SHM_CONNECTION_H
#include "RingBuffer.h"
#include "Util.h"
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#define get_idx(src, dst) src * 65536 + dst
#define POOL_SIZE 16

typedef struct Connection {	
	uint32_t src;
	uint32_t dst;
	RingBuffer *rb;
	bool idle;
} Connection_t;

typedef struct Pool {
	Connection_t *connections;
	uint32_t size;
	uint32_t capacity;
	List_t *idle_list;
} Pool_t;

Pool_t *pool_init(int size);

static inline void expandPool(Pool_t *pool) {
	uint32_t new_size = pool->capacity * 2;
	pool->connections = (Connection_t *)realloc(pool->connections,
				sizeof(Connection_t) * new_size);
	for (int i = pool->size; i < new_size; i++) {
		pool->connections[i].idle = true;
	}
	pool->capacity = new_size;
}

int conn_open(uint32_t src_port, uint32_t dst_port);
Connection_t *conn_create(Pool_t *pool);
void conn_remove(Pool_t *pool, uint32_t src_port, uint32_t dst_port);
static inline int conn_read(Connection_t *conn, int len, char *output) {
	return rb_read(conn->rb, len, output);
}

static inline int conn_write(Connection_t *conn, int len, char *input) {
	return rb_write(conn->rb, len, input);
}

#endif
