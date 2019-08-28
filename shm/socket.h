#ifndef __SOCKET_H
#define __SOCKET_H
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h> 

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
//struct hostent *gethostbyname(const char *name);
//struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);

#endif
