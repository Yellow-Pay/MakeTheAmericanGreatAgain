#include <signal.h>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <future>
#include <vector>
#include <numeric>
#include <iostream>
#include <stdint.h>
#include <sys/wait.h>
#include "TestUtil.h"

using std::cout;
using std::vector;
using std::endl;

static inline uint64_t get_time() {
	uint32_t lo, hi;
	__asm__ volatile("rdtsc\n\t"
		:"=a"(lo), "=d"(hi)
		:
		:);
	return (uint64_t)hi << 32 | lo;
}

int server_port = 0;

auto add_future = [](int lhs, std::future<int>& rhs) -> int {
    return lhs + rhs.get();
};

void server() {
	auto start = get_time();
	const int thread_number = server_thread_number;
    int server_fd, new_socket; 
	struct sockaddr_in address; 
	int addrlen = sizeof(address); 

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 

	// Forcefully attaching socket to the port 8080 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( server_port ); 

	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, 
					sizeof(address)) < 0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
						(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	} 
	printf("address port: %d\n", address.sin_port);
    std::vector<std::future<int>> f(thread_number);
    for (int i = 0; i < thread_number; ++i) {
        f[i] = std::async([=]() -> int {
	        char buffer[1024] = {0}; 
            int ans = 0;
            for (int i = 0; i < write_read_number; ++i) {
    	        int valread = recv(new_socket, buffer, 1024, 0); 
                ans += valread;
            }
            return ans;
        });
    }
    int total_read = std::accumulate(f.begin(), f.end(), 0, add_future);
	auto end = get_time();
    cout << "\n[servers, clients, wrnumbers] = [" << server_thread_number << "," 
	    << client_thread_number << "," << write_read_number << "] Server: Total read bytes: " << total_read << ", cost: " << end - start << endl;
    sleep(5);
	close(new_socket);
}

int main(int argc, char const *argv[])
{
	signal(SIGPIPE, SIG_IGN);
    assert(argc == 2);
    server_port = atoi(argv[1]);
    server();

    return 0;
}
