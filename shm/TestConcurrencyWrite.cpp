/**
 * Test Concurrency read and write using socket API.
 * This file is for write end: client.
 * Another file "TestConcurrencyRead.cpp" is for read end: server.
 */
#include "TestUtil.h"
#include <arpa/inet.h>
#include <cassert>
#include <future>
#include <iostream>
#include <numeric>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using std::cout;
using std::endl;
using std::vector;

static inline uint64_t get_time() {
    uint32_t lo, hi;
    __asm__ volatile("rdtsc\n\t" : "=a"(lo), "=d"(hi) : :);
    return (uint64_t)hi << 32 | lo;
}

int server_port = 0;

auto add_future = [](int lhs, std::future<int> &rhs) -> int {
    return lhs + rhs.get();
};

int client() {
    auto start = get_time();
    const int thread_number = client_thread_number;
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    std::vector<std::future<int>> f(thread_number);
    char buffer[1024];
    memset(buffer, 0x3f, 1024);
    for (int i = 0; i < thread_number; ++i) {
        f[i] = std::async([=]() -> int {
            int ans = 0;
            for (int i = 0; i < write_read_number; ++i) {
                int valread = send(sock, buffer, 1024, 0);
                ans += valread;
            }
            return ans;
        });
    }
    int total_write = std::accumulate(f.begin(), f.end(), 0, add_future);
    auto end = get_time();
    cout << "\n[servers, clients, wrnumbers] = [" << server_thread_number << ","
         << client_thread_number << "," << write_read_number
         << "] Client: Total send bytes: " << total_write
         << ", cost: " << end - start << endl;
    sleep(5);
    close(sock);
    return 0;
}

int main(int argc, char const *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    assert(argc == 2);
    server_port = atoi(argv[1]);
    client();

    return 0;
}
