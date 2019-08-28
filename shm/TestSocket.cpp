#include "socket.h"
#include <arpa/inet.h>
#include <cassert>
#include <future>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int server_port = 0;

void server() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server_port);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("address port: %d", address.sin_port);
    valread = read(new_socket, buffer, 1024);
    printf("%s\n", buffer);
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent from server.\n");
    close(new_socket);
}

int client() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
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
    send(sock, hello, strlen(hello), 0);
    printf("Hello message sent from client\n");
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);
    sleep(10);
    close(sock);
    return 0;
}

int main(int argc, char const *argv[]) {
    assert(argc == 2);
    server_port = atoi(argv[1]);
    int pid = fork();
    if (pid == 0) {
        server();
    } else {
        sleep(3);
        const int thread_number = 1;
        std::future<int> ret[thread_number];
        for (int i = 0; i < thread_number; ++i) {
            ret[i] = std::async(client);
        }
        // 最多有一个连接成功，如果server失败，则0个连接成功
        bool success = false;
        for (int i = 0; i < thread_number; ++i) {
            if (ret[i].get() == 0) {
                assert(success == false);
                success = true;
            }
        }
    }
    return 0;
}
