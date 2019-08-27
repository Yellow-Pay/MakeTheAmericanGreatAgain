#include <stddef.h>
#include <sys/types.h>
#include "socket.h"

int socket(int domain, int type, int protocal) {
	int ret = 0;
	// TODO:
	return ret;
}

int bind(int fd, const struct sockaddr *addr, socklen_t len) {
//	return socketcall(bind, fd, addr, len, 0, 0, 0);
}

int listen(int fd, int backlog) {
//	return socketcall(listen, fd, backlog, 0, 0, 0, 0);
}

int connect(int fd, const struct sockaddr *addr, socklen_t len) {
//	return socketcall_cp(connect, fd, addr, len, 0, 0, 0);
}

int accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict len) {
//	return socketcall_cp(accept, fd, addr, len, 0, 0, 0);
}

ssize_t send(int fd, const void *buf, size_t len, int flags) {
//	return sendto(fd, buf, len, flags, 0, 0);
}

ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t alen) {
//	return socketcall_cp(sendto, fd, buf, len, flags, addr, alen);
}

ssize_t recv(int fd, void *buf, size_t len, int flags) {
	return recvfrom(fd, buf, len, flags, 0, 0);
}

ssize_t recvfrom(int fd, void *restrict buf, size_t len, int flags, struct sockaddr *restrict addr, socklen_t *restrict alen) {
//	return socketcall_cp(recvfrom, fd, buf, len, flags, addr, alen);
}

int close(int fd) {
	// TODO:
}
