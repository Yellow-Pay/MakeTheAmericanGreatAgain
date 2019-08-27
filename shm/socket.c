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

#define GET_SERVER_PORT_FROM_FD(x) ((x)&0xFFFF)
#define GET_CLIENT_PORT_FROM_FD(x) ((x) >> 16)
#define FD2PORT(fd) ((fd)*65536 + getpid())

#define LISTEN_PID_TABLE_KEY (0x4f4f4f4f)
#define PORT_NUMBER (65536)
int port_table_availble = 0;
int *port_table_address = 0;

int client_fd_to_server_fd[1024];

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

int read_port_table(int port) {
    if (port_table_availble == 0) {
        // TODO: multithread support
        init_port_table();
    }
    assert(port >= 0);
    assert(port < PORT_NUMBER);
    return port_table_address[port];
}

/*
make the fd of the sender the port of sender,
make the fd of the reciver the port of reciver.
*/
//int socket(int domain, int type, int protocol) {}

int bind(int fd, const struct sockaddr *addr, socklen_t len) {
    struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
    int port = addr_in->sin_port;
    if (0 != read_port_table(port)) {
        // current port is unavailable
        return -1;
    }
    write_port_table(port, getpid());
    return 0;
}

int listen(int fd, int backlog) {
    //	return socketcall(listen, fd, backlog, 0, 0, 0, 0);
    return 0;
}

int connect(int fd, const struct sockaddr *addr, socklen_t len) {
    //	return socketcall_cp(connect, fd, addr, len, 0, 0, 0);
    struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
    int port = addr_in->sin_port;
    int pid = read_port_table(port);
    write_port_table(port, FD2PORT(fd));
    client_fd_to_server_fd[fd] = port;
    // send signal to pid
    kill(pid, SIGUSR1);
    return 0;
}

void accept_handler(int sig, siginfo_t *siginfo, void *context) { return; }

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
    int client_port = addr_in->sin_port;
    return (client_port << 16) + server_port;
}

ssize_t send(int fd, const void *buf, size_t len, int flags) {
    return sendto(fd, buf, len, flags, 0, 0);
}

ssize_t sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *addr, socklen_t alen) {
    RingBuffer_t *writer = NULL;
    if (fd > 1024) { // server
        writer = rb_get(
            get_idx(GET_SERVER_PORT_FROM_FD(fd), GET_CLIENT_PORT_FROM_FD(fd)));
    } else { // client
        writer = rb_get(get_idx(FD2PORT(fd), client_fd_to_server_fd[fd]));
    }
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
    if (fd > 1024) { // server
        reader = rb_get(
            get_idx(GET_CLIENT_PORT_FROM_FD(fd), GET_SERVER_PORT_FROM_FD(fd)));
    } else { // client
        reader = rb_get(get_idx(client_fd_to_server_fd[fd], FD2PORT(fd)));
    }
    return rb_read(reader, len, (char *)buf);
}

int close(int fd) {
    write_port_table(GET_CLIENT_PORT_FROM_FD(fd), 0);
    write_port_table(GET_SERVER_PORT_FROM_FD(fd), 0);
    RingBuffer_t *reader = NULL;
    if (fd > 1024) { // server
        reader = rb_get(
            get_idx(GET_CLIENT_PORT_FROM_FD(fd), GET_SERVER_PORT_FROM_FD(fd)));
    } else { // client
        reader = rb_get(get_idx(client_fd_to_server_fd[fd], FD2PORT(fd)));
    }
    rb_destroy(reader);
    return 0;
}
