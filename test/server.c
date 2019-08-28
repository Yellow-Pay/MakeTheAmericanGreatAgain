#include "test-util.h"
#include <string.h>
#include <stdio.h>
//#include "socket.h"
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#define PORT 9999
int server() {
	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	memset(buffer, 'a', 1023);
	buffer[1023] = 0;
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	}
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	printf("port = %d\n", address.sin_port);

	if (bind(server_fd, (struct sockaddr *)&address,  
					sizeof(address))<0) 
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
	uint64_t begin, end;
	begin = get_time();
	for (int i = 0; i < MICRO_BENCHMARK_TESTNUMS; i++) {
		send(new_socket, buffer, 1024 , 0 ); 
	}
	end = get_time();
	printf("[MICRO_SOCKET] %lu WRITE_OPS takes %lu CPU cycles\n", MICRO_BENCHMARK_TESTNUMS, end - begin);
	sleep(20);
	close(server_fd);
	return 0;
}

int main(int argc, char *argv[]) {
	server();
	return 0;
}
