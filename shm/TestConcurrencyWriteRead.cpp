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

const int server_thread_number = 4;
const int client_thread_number = 4;
const int write_read_number = 10;
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
	close(new_socket);
	auto end = get_time();
    cout << "\nServer: Total read bytes: " << total_read << ", cost: " << end - start << endl;
}

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
	close(sock);
	auto end = get_time();
    cout << "\nClient: Total send bytes: " << total_write << ", cost: " << end - start << endl;
	return 0;
}

int main(int argc, char const *argv[])
{
    assert(argc == 2);
    server_port = atoi(argv[1]);
    int pid = fork();
	if (pid == 0) {
		server();
	} else {
		int pid2 = fork();
		if (pid2 == 0) {
			sleep(1);
			client();
		} else {
			waitpid(pid, 0, 0);
			waitpid(pid2, 0, 0);
		}
	}
    return 0;
}
