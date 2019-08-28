// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include "socket.h"
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <assert.h>

int main(int argc, char const *argv[]) 
{ 
	assert(argc == 2);
	int port = atoi(argv[1]);
	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char *hello = "Hello from server"; 

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 

	// Forcefully attaching socket to the port 8080 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( port ); 

	// Forcefully attaching socket to the port 8080 
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
	printf("address port: %d\n", address.sin_port);
	sleep(3);
	valread = recv(new_socket , buffer, 1024, 0); 
	printf("read success: %d\n", valread);
	printf("%s\n",buffer ); 
	valread = send(new_socket , hello , strlen(hello) , 0 ); 
	printf("send bytes: %d\n", valread);
	printf("Hello message sent\n"); 
	// sleep(10);
	close(new_socket);
	return 0; 
} 
