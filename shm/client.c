// Client side C/C++ program to demonstrate Socket programming
#include "socket.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    assert(argc == 2);
    int PORT = atoi(argv[1]);
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    int ret;
    ret = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        printf("\nConnection Failed %d\n", ret);
        return -1;
    }
    ret = send(sock, hello, strlen(hello), 0);
    printf("Hello message sent: %d\n", ret);
    sleep(6);
    valread = recv(sock, buffer, 1024, 0);
    printf("Read successful bytes: %d\n", valread);
    printf("%s\n", buffer);
    // sleep(5);
    close(sock);
    return 0;
}
