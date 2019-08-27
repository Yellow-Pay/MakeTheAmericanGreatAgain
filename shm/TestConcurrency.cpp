#include "Connection.h"
#include "RingBuffer.h"
#include <cassert>
#include <chrono>
#include <cstring>
#include <future>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

const int thread_number = 100;
void sleep_random_time(int left = 10, int right = 100) {
    std::mt19937_64 eng{std::random_device{}()}; // or seed however you want
    std::uniform_int_distribution<> dist{left, right};
    std::this_thread::sleep_for(std::chrono::milliseconds{dist(eng)});
}

void read_concurrency() {
    Connection c(8009, 8001);
    vector<std::future<int>> v(thread_number);
    for (int i = 0; i < thread_number; ++i) {
        v[i] = std::async([&c, i]() -> int {
            sleep_random_time();
            char buf[100];
            memset(buf, 0, 100);
            auto ret = c.read(1, buf);
            cout << "read: " << int(buf[0]) << endl;
            assert(buf[1] == '\0');
            return ret;
        });
    }
}
void write_concurrency() {
    Connection c(8001, 8009);
    vector<std::future<int>> v(thread_number);
    for (int i = 0; i < thread_number; ++i) {
        v[i] = std::async([&c, i]() -> int {
            char buf = i;
            sleep_random_time();
            auto ret = c.write(1, &buf);
            cout << "write: " << i << endl;
            return ret;
        });
    }
}

/*
Test read and write to the end of ringbuffer and read and write too much
*/
void TestNonConcurrency() {
    Connection writer(8009, 8001);
    Connection reader(8001, 8009);
    char buf[100];
    int ret = writer.write(26, "abcdefghijklmnopqrstuvwxyz");
    cout << "ret: " << ret << endl;
    assert(ret == SHM_SIZE - 3 * sizeof(uint32_t) - 1);
    memset(buf, 0, 100);
    ret = reader.read(10, buf);
    assert(ret == 10);
    cout << "buf: " << buf << endl;
    assert(strcmp(buf, "abcdefghij") == 0);
    ret = writer.write(10, "0123456789");
    assert(ret == 10);
    memset(buf, 0, 100);
    assert(19 == reader.read(26, buf));
    cout << "Data read in memory: " << buf << endl;
    assert(strcmp(buf, "klmnopqrs0123456789") == 0);
}
void TestConcurrency() {
    auto a = std::async(write_concurrency);
    auto b = std::async(read_concurrency);
}
void onlyWrite() {
    Connection writer(8008, 8000);
    for (int i = 0; i < 50; ++i) {
        char buf = 'a';
        int ret = writer.write(1, &buf);
        cout << "write " << i << ", " << ret << endl;
    }
}
int main() {
    TestConcurrency();
    return 0;
}
