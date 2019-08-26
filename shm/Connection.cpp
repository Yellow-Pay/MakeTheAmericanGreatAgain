#include "Connection.h"
#include <cstring>
#include <cassert>
#include <map>

using namespace std;

Pool_t *ConnectionPool;
map<uint32_t, Connection_t *> ConnectionMap;
//TODO: shared map

int conn_open(uint32_t src_port, uint32_t dst_port) {
	if (!ConnectionPool) {
		ConnectionPool = pool_init(POOL_SIZE); 
	}
	uint32_t idx = get_idx(src_port, dst_port);
	if (ConnectionMap.find(idx) != ConnectionMap.end()) {
		// Already connected
		return -1;
	}
	Connection_t *conn = conn_create(ConnectionPool);
	ConnectionMap[idx] = conn;
	conn->idle = false;
	return 0;
}

Connection_t *conn_create(Pool_t *pool) {
	Connection_t *conn = NULL;
	uint32_t size = pool->size;
	uint32_t capacity = pool->capacity;
	if (size == capacity) {
		expandPool(pool);
	}
	uint32_t idx = size;
	if (!list_empty(pool->idle_list)) {
		idx = list_front(pool->idle_list);
		list_pop(pool->idle_list);
	}
	conn = &pool->connections[idx];
	if (!conn->rb) {
		conn->rb = rb_init(idx);
	}
	pool->size++;
	return conn;
}

void conn_remove(Pool_t *pool, uint32_t src_port, uint32_t dst_port) {
	uint32_t idx = get_idx(src_port, dst_port);
	assert(ConnectionMap.find(idx) != ConnectionMap.end());
	Connection_t *conn = ConnectionMap[idx];
	conn->idle = true;
	uint32_t idle_idx = (conn - ConnectionPool->connections);
	list_push(pool->idle_list, idle_idx);
	ConnectionMap.erase(idx);
}

Pool_t *pool_init(int size) {
	Pool_t *pool = (Pool_t *)malloc(sizeof(Pool_t));
	if (!pool) return NULL;
	pool->connections = (Connection_t *)malloc(sizeof(Connection_t) * size);
	if (!pool->connections) {
		free(pool);
		return NULL;
	}
	pool->idle_list = (List_t *)malloc(sizeof(List_t));
	if (!pool->idle_list) {
		free(pool->connections);
		free(pool);
		return NULL;
	}
	memset(pool->connections, 0, sizeof(Connection_t) * size);
	memset(pool->idle_list, 0, sizeof(List_t));
	for (int i = 0; i < size; i++) {
		pool->connections[i].idle = true;
	}
	pool->size = 0;
	pool->capacity = size;
	return pool;
}
