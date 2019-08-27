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
#include "RingBuffer.h"

using namespace std;

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
