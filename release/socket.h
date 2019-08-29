/**
 * The same API with <sys/socket.h>
 * We implement all of them using shared memory.
 * Users could include this or <sys/socket.h>.
 * 
 * Abstract layer:
 * 
 * | Socket API | Connection API |
 * | RingBuffer |     Pool       |
 * |        shared memory        |
 */
#ifndef __SOCKET_H
#define __SOCKET_H
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
int close(int fd);
#ifdef __cplusplus
}
#endif

#endif
