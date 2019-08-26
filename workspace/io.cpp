#include <cstring>
#include <iostream>
#include <map>
#include <cassert>
#include <queue>
#include <sys/shm.h>
#include <cstdint>

using namespace std;

#define SHM_SIZE 32
#define get_idx(srcPort, destPort) srcPort * 65536 + destPort
#define SHM_DATA_SIZE (SHM_SIZE - 2 * sizeof(char*))

struct RingBuffer {
	int shmid;
	char* address;
	char* content;
	RingBuffer(key_t key) {
		shmid = shmget(key, SHM_SIZE, 0666|IPC_CREAT);
		shmid_ds info;
		shmctl(shmid, IPC_STAT, &info);
		address = (char*) shmat(shmid, (void*)0, 0);
		char** data = (char**)address;
		content = (char*)&data[2];
		if (info.shm_nattch == 0) {
			data[0] = content;
			data[1] = content;
		}
	}
	void destroy() {
		// destroy the shared memory 
    	shmctl(shmid,IPC_RMID,NULL); 
	}
	~RingBuffer() {
		shmdt(address);
		shmid_ds info;
		shmctl(shmid, IPC_STAT, &info);
		if (info.shm_nattch == 0) {
    		shmctl(shmid,IPC_RMID,NULL); 
		}
	}
	/* Head, Tail, content */
	void movePointer(uint32_t len, int index) {
		char** data = (char**) address;
		data[index] += len;
		if (data[index] > content + SHM_DATA_SIZE) {
			data[index] -= SHM_DATA_SIZE;
		}
	}
	void* getPointer(int index) {
		char** data = (char**) address;
		return data[index];
	}
	void* getHead() {
		return getPointer(0);
	}
	void moveHead(uint32_t len) {
		movePointer(len, 0);
	}
	void* getTail() {
		return getPointer(1);
	}
	void moveTail(uint32_t len) {
		movePointer(len, 1);	
	}

	int read(int len, char *output) {
		char* head = (char*)getHead();
		char* tail = (char*)getTail();
		int size = (tail > head) ? (tail - head) : (tail + SHM_DATA_SIZE - head);
		if (size < len) {
			len = size;
		}
		if (head + len < content + SHM_DATA_SIZE) {
			memcpy(output, head, len);
		} else {
			int remain_length = SHM_DATA_SIZE - (head - content);
			memcpy(output, head, remain_length);
			memcpy(output + remain_length, content, len - remain_length);
		}
		moveHead(len);
		// TODO: add multi thread support
		return len;
	}

	int write(int len, const char *input) {
		char* head = (char*)getHead();
		char* tail = (char*)getTail();
		int size = (head > tail) ? (head - tail) : (head + SHM_DATA_SIZE - tail);
		if (size < len) {
			len = size;
		}
		if (tail + len < content + SHM_DATA_SIZE) {
			memcpy(tail, input, len);
		} else {
			int remain_length = SHM_DATA_SIZE - (tail - content);
			memcpy(tail, input, remain_length);
			memcpy(content, input, len - remain_length);
		}
		moveTail(len);
		// TODO: add multi thread support
		return len;
	}
};

typedef struct Connection {
	uint32_t src;
	uint32_t dst;
	RingBuffer readRB, writeRB;
	Connection(uint32_t src_port, uint32_t dst_port) : src(src_port), dst(dst_port), readRB(get_idx(src, dst)), writeRB(get_idx(dst, src)) {
	}
	~Connection() {
		// 读者负责关
		readRB.destroy();
	}
	int read(int len, char *output) {
		return readRB.read(len, output);
	}
	int write(int len, const char *input) {
		return writeRB.write(len, input);
	}

	bool idle;
	void *address;
	uint64_t length;
} Connection_t;

void testConnection(int argc, char* argv[]) {
	assert(argc == 4);
	uint32_t src = atoi(argv[1]), dst = atoi(argv[2]);
	Connection c(src, dst);
	Connection c_reverse(dst, src);
	char buf[100];
	cout << "write success: " << c.write(12, "abcdefghijkl") << endl;
	memset(buf, 0, 100);
	cout << "read success: " << c_reverse.read(5, buf) << endl;
    cout << "Data read in memory: " << buf << endl; 
    cout << "Data written in memory: " << argv[3] << endl; 
	cout << "write success: " << c.write(5, argv[3]) << endl;
	memset(buf, 0, 100);
	cout << "read success: " << c_reverse.read(12, buf) << endl;
    cout << "Data read in memory: " << buf << endl;
}

typedef struct Pool {
	Connection_t *connections;
	uint32_t size;
	uint32_t capacity;
	queue<uint32_t> idle_list;
} Pool_t;

Pool_t *init(int size) {
	Pool_t *pool = new Pool_t();
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

Connection_t *addConnection(Pool_t *pool, uint32_t key) {
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

int openConnection(uint32_t srcPort, uint32_t destPort) {
	uint32_t idx = get_idx(srcPort, destPort);
	if (ConnectionMap.find(idx) != ConnectionMap.end()) {
		// Already connected
		return -1;
	}
	Connection_t *conn = addConnection(ConnectionPool, idx);
	ConnectionMap[idx] = conn;
	conn->idle = false;
	return 0;
}

int closeConnection(uint32_t srcPort, uint32_t destPort) {
	uint32_t idx = get_idx(srcPort, destPort);
	if (ConnectionMap.find(idx) == ConnectionMap.end()) {
		return -1;
	}
	Connection_t *conn = ConnectionMap[idx];
	conn->idle = true;
	shmdt(conn->address);
	uint32_t idle_idx = (conn - ConnectionPool->connections);
	ConnectionPool->idle_list.push(idle_idx);
	ConnectionMap.erase(idx);
	return 0;
}

int main(int argc, char* argv[]) {
	testConnection(argc, argv);
	// ConnectionPool = init(8);
	// ConnectionPool->idle_list.push(0);
	// assert(openConnection(1, 2) == 0);
	// assert(openConnection(2, 1)  == 0);
	// assert(openConnection(3, 2)  == 0);
	// closeConnection(2, 1);
	return 0;
}
