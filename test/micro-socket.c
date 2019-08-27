#include "test-util.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
int PORT;

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
		send(new_socket , buffer, 1024 , 0 ); 
	}
	end = get_time();
	printf("[MICRO_SOCKET] %lu WRITE_OPS takes %lu CPU cycles\n", MICRO_BENCHMARK_TESTNUMS, end - begin);
	sleep(10);
	return 0;
}

int client() {
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *hello = "Hello from client"; 
	char buffer[1024] = {0}; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 
	uint64_t begin, end;
	begin = get_time();
	for (int i = 0; i < MICRO_BENCHMARK_TESTNUMS; i++) {
		recv(sock, buffer, 1024, 0);
	}
	end = get_time();
	printf("[MICRO_SOCKET] %lu READ_OPS takes %lu CPU cycles\n", MICRO_BENCHMARK_TESTNUMS, end - begin);
	sleep(10);
	return 0; 
}

int main(int argc, char *argv[]) {
	PORT = atoi(argv[1]);
	int pid = fork();
	if (pid == 0) {
		server();
	} else {
		sleep(3);
		client();
	}
	wait(NULL);
	return 0;
}
