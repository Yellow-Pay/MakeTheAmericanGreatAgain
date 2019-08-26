#include <future>
#include "RingBuffer.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

using namespace std;

const int thread_number = 100;

void sleep_random_time() {
    std::mt19937_64 eng{std::random_device{}()};  // or seed however you want
    std::uniform_int_distribution<> dist{10, 100};
    std::this_thread::sleep_for(std::chrono::milliseconds{dist(eng)});
}

void read_concurrency(RingBuffer_t *rb) {
    vector<std::future<int>> v(thread_number);
    for (int i = 0; i < thread_number; ++i) {
        v[i] = std::async([=]() -> int {
            sleep_random_time();
            char buf[100];
            memset(buf, 0, 100);
            auto ret = rb_read(rb, 1, buf);
            cout << "read: " << int (buf[0]) << endl;
            assert(buf[1] == '\0');
            return ret;
        });
    }
}
void write_concurrency(RingBuffer_t *rb) {
    vector<std::future<int>> v(thread_number);
    for (int i = 0; i < thread_number; ++i) {
        v[i] = std::async([=]() -> int {
            char buf = i;
            sleep_random_time();
            auto ret = rb_write(rb, 1, &buf);
            cout << "write: " << i << endl;
            return ret;
        });
    }
}

void TestNonConcurrency() {
    auto writer = rb_init(1234);
    auto reader = rb_init(1234);
    char buf[100];
	cout << "write success: " << rb_write(writer, 12, "abcdefghijkl") << endl;
	memset(buf, 0, 100);
	cout << "read success: " << rb_read(reader, 5, buf) << endl;
	cout << "Data read in memory: " << buf << endl; 
	cout << "write success: " << rb_write(writer, 5, "12345") << endl;
	memset(buf, 0, 100);
	cout << "read success: " << rb_read(reader, 12, buf) << endl;
	cout << "Data read in memory: " << buf << endl;
}
void TestConcurrency() {
    auto writer = rb_init(1234);
    auto reader = rb_init(1234);
    std::async(write_concurrency, writer);
    std::async(read_concurrency, reader);
}
void onlyWrite() {
    auto writer = rb_init(1234);
    for (int i = 0; i < 50; ++i) {
        char buf = 'a';
        int ret = rb_write(writer, 1, &buf);
        cout << "write " << i << ", " << ret << endl;
    }
}
int main() {
    TestConcurrency();
    return 0;
}