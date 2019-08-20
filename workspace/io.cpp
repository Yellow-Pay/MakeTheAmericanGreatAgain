#include <string.h>
#include <iostream>
#include <map>
#include <queue>
using namespace std;

typedef struct Connection {
	bool idle;
	void *address;
	uint64_t length;
} Connection_t;

typedef struct Pool {
	Connection_t *connections;
	uint32_t size;
	uint32_t capacity;
	queue<uint32_t> idle_list;
} Pool_t;

Pool_t *init(int size) {
	Pool_t *pool = (Pool_t *)malloc(sizeof(Pool_t));
	if (!pool) return NULL;
	pool->connections = (Connection_t *)malloc(sizeof(Connection_t) * size);
	memset(pool->connections, 0, sizeof(Connection_t) * size);
	if (!pool->connections) {
		free(pool);
		return NULL;
	}
	for (int i = 0; i < size; i++) {
		pool->connections[i].idle = true;
	}
	pool->size = 0;
	pool->capacity = size;
	return pool;
}

static inline void expandPool(Pool_t *pool) {
	uint32_t new_size = pool->capacity * 2;
	pool->connections = (Connection_t *)realloc(pool->connections, 
				sizeof(Connection_t) * new_size);
	for (int i = pool->size; i < new_size; i++) {
		pool->connections[i].idle = true;
	}
	pool->capacity = new_size;
}

Connection_t *addConnection(Pool_t *pool) {
	Connection_t *conn = NULL;
	uint32_t size = pool->size;
	uint32_t capacity = pool->capacity;
	if (size == capacity) {
		expandPool(pool);
	}
	uint32_t idx = size;
	if (!pool->idle_list.empty()) {
		idx = pool->idle_list.front();
		pool->idle_list.pop();
	}
	conn = &pool->connections[idx];
	if (!conn->address) {
		// TODO: map a region of memory
	}
	pool->size++;
	return conn;
}

Pool_t *ConnectionPool;
map<int, Connection_t *> ConnectionMap;

int openConnection(uint32_t port) {
	uint32_t idx = port;
	if (ConnectionMap.find(idx) != ConnectionMap.end()) {
		// Already connected
		return -1;
	}
	Connection_t *conn = addConnection(ConnectionPool);
	ConnectionMap[idx] = conn;
	conn->idle = false;
	return 0;
}

int closeConnection(uint32_t port) {
	uint32_t idx = port;
	if (ConnectionMap.find(idx) == ConnectionMap.end()) {
		return -1;
	}
	Connection_t *conn = ConnectionMap[idx];
	conn->idle = true;
	uint32_t idle_idx = (conn - ConnectionPool->connections);
	ConnectionPool->idle_list.push(idle_idx);
	ConnectionMap.erase(idx);
	return 0;
}

int main() {
	ConnectionPool = init(8);
	openConnection(1);
	openConnection(2);
	openConnection(3);
	closeConnection(2);
	return 0;
}
