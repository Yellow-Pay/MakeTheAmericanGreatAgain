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
	sleep(10);
	uint64_t begin, end;
	begin = get_time();
	for (int i = 0; i < MICRO_BENCHMARK_TESTNUMS; i++) {
		recv(sock, buffer, 1024, 0);
	}
	end = get_time();
	printf("[MICRO_SOCKET] %lu READ_OPS takes %lu CPU cycles\n", MICRO_BENCHMARK_TESTNUMS, end - begin);
	close(sock);
	return 0; 
}

int main(int argc, char *argv[]) {
	client();
	return 0;
}
