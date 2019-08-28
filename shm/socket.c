#include "socket.h"
#include "RingBuffer.h"
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#define MAX_PID_BITS (22)
#define GET_LOCAL_PORT_FROM_FD(x) ((x)&0xFFFF)
#define GET_REMOTE_PORT_FROM_FD(x) (((x) >> 16) & 0xFFFF)
#define FD2PORT(fd) (((fd) << MAX_PID_BITS) | getpid())

#define LISTEN_PID_TABLE_KEY (0x4f4f4f4f)
#define PORT_NUMBER (65536)
int port_table_availble = 0;
int *port_table_address = 0;

int client_fd_to_idx[1024]; // fd - RemotePort << 16 | LocalPort

void init_port_table() {
	int shmid = shmget(LISTEN_PID_TABLE_KEY, PORT_NUMBER * sizeof(int),
				0666 | IPC_CREAT);
	shmid_ds info;
	shmctl(shmid, IPC_STAT, &info);
	port_table_address  = (int *)shmat(shmid, (void *)0, 0);
	if (info.shm_nattch == 0) {
		memset(port_table_address, 0, PORT_NUMBER * sizeof(int));
	}
	port_table_availble = 1;
}

void write_port_table(int port, int value) {
	if (port_table_availble == 0) {
		// TODO: multithread support
		init_port_table();
	}
	port_table_address[port] = value;
}

static inline int* read_port_table_address(int port) {
	return &port_table_address[port];
}

int read_port_table(int port) {
	if (port_table_availble == 0) {
		// TODO: multithread support
		init_port_table();
	}
	assert(port >= 0);
	assert(port < PORT_NUMBER);
	return port_table_address[port];
}

int get_free_port(int idx) {
	for (int i = 1; i < PORT_NUMBER; i++) {
		if (0 == read_port_table(i)) {
			write_port_table(i, idx);
			return i;
		}
	}
	return 0;
}

/*
   make the fd of the sender the port of sender,
   make the fd of the reciver the port of reciver.
   */
//int socket(int domain, int type, int protocol) {}

int bind(int fd, const struct sockaddr *addr, socklen_t len) {
	//printf("bind: \n");
	struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
	int port = addr_in->sin_port;
	if (0 != read_port_table(port)) {
		// current port is unavailable
		return -1;
	}
	write_port_table(port, getpid());
	//printf("socket bind API: port = %d, write_port_table: %d\n", port, read_port_table(port));
	return 0;
}

int listen(int fd, int backlog) {
	//	return socketcall(listen, fd, backlog, 0, 0, 0, 0);
	return 0;
}

int connect(int fd, const struct sockaddr *addr, socklen_t len) {
	//	return socketcall_cp(connect, fd, addr, len, 0, 0, 0);
	struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
	int server_port = addr_in->sin_port;
	uint32_t pid, idx;
	pid = read_port_table(server_port);
#ifndef NDEBUG
	//printf("socket connet API: server pid: %u\n", pid);
#endif
	if (pid >= (1 << MAX_PID_BITS)) {	// the remote port is connected by another client
		return  -1;
	}
	if (pid == 0) { // the remote is not open
		return -2;
	}
	int client_port = get_free_port(FD2PORT(fd));
	if (client_port == 0) return -1;
	idx = (server_port << 16) | client_port;
	if (!__sync_bool_compare_and_swap(read_port_table_address(server_port), pid, (client_port | (1 << MAX_PID_BITS)))) {
	//	printf("Failed - server_port = %d\n", server_port);
	//	printf("pid = %d, ans = %d\n", pid, read_port_table(server_port));
		write_port_table(client_port, 0);
		return -1;
	}
	client_fd_to_idx[fd] = idx;
	if (!rb_get(get_idx(GET_REMOTE_PORT_FROM_FD(idx), GET_LOCAL_PORT_FROM_FD(idx)))) {
		return -3;
	}
	// send signal to pid
	kill(pid, SIGUSR1);
	return 0;
}

void accept_handler(int sig, siginfo_t *siginfo, void *context) { 
	return; 
}

// int accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict len) {
int accept(int fd, struct sockaddr *addr, socklen_t *len) {
	struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
	int server_port = addr_in->sin_port;
	struct sigaction siga;
	// prepare sigaction
	siga.sa_sigaction = accept_handler;
	siga.sa_flags |= SA_SIGINFO; // get detail info

	sigaction(SIGUSR1, &siga, NULL);
	pause();
	// wait for signal
	if (!rb_get(get_idx(GET_REMOTE_PORT_FROM_FD(fd), GET_LOCAL_PORT_FROM_FD(fd)))) {
		return -1;
	}
	int client_port = read_port_table(server_port) & 0xFFFF;
	return (client_port << 16) | server_port;
}

ssize_t send(int fd, const void *buf, size_t len, int flags) {
	return sendto(fd, buf, len, flags, 0, 0);
}

ssize_t sendto(int fd, const void *buf, size_t len, int flags,
			const struct sockaddr *addr, socklen_t alen) {
	RingBuffer_t *writer = NULL;
	if (fd < 1024) {
		fd = client_fd_to_idx[fd];
	}
	writer = rb_get(get_idx(GET_LOCAL_PORT_FROM_FD(fd), GET_REMOTE_PORT_FROM_FD(fd)));
	if (!writer) return -1;
#ifndef NDEBUG
	printf("socket send API: writer->index: %d, writer: %x, address: %x\n", writer->index, writer, writer->address);
#endif	
	return rb_write(writer, len, (char *)buf);
}

ssize_t recv(int fd, void *buf, size_t len, int flags) {
	return recvfrom(fd, buf, len, flags, 0, 0);
}

// ssize_t recvfrom(int fd, void *restrict buf, size_t len, int flags,
//                  struct sockaddr *restrict addr, socklen_t *restrict alen) {
ssize_t recvfrom(int fd, void *buf, size_t len, int flags,
			struct sockaddr *addr, socklen_t *alen) {
	RingBuffer_t *reader = NULL;
	if (fd < 1024) {
		fd = client_fd_to_idx[fd];
	}
	reader = rb_get(get_idx(GET_REMOTE_PORT_FROM_FD(fd), GET_LOCAL_PORT_FROM_FD(fd)));
	if (!reader) return -1;
#ifndef NDEBUG
	printf("socket recv API: reader->index: %d, reader: %x, address: %x\n", reader->index, reader, reader->address);
#endif
	int ret = rb_read(reader, len, (char *)buf);
	while (ret == 0) {
		if (read_port_table(GET_REMOTE_PORT_FROM_FD(fd)) == 0) {
			ret = rb_read(reader, len, (char *)buf);
			return ret;
		}
		ret = rb_read(reader, len, (char *)buf);
	}
	return ret;
}

int close(int fd) {
	if (fd < 1024) {
		fd = client_fd_to_idx[fd];
	}
	write_port_table(GET_LOCAL_PORT_FROM_FD(fd), 0);
	RingBuffer_t *reader = NULL;
	reader = rb_get(get_idx(GET_REMOTE_PORT_FROM_FD(fd), GET_LOCAL_PORT_FROM_FD(fd)));
	if (!reader) return -1;
#ifndef NDEBUG
	printf("socket close API: fd = %d, src: %d, dst: %d, reader->index: %d\n", fd, GET_REMOTE_PORT_FROM_FD(fd), GET_LOCAL_PORT_FROM_FD(fd), reader->index);
#endif
	clear_idx(GET_REMOTE_PORT_FROM_FD(fd), GET_LOCAL_PORT_FROM_FD(fd));
	rb_destroy(reader);
	return 0;
}
