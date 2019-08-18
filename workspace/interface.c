typedef u_short sa_family_t;
struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};
// socket() creates an endpoint for communication and 
// returns a file descriptor that refers to that endpoint.
int socket(int domain, int type, int protocol);

// bind() assigns the address specified by addr to the socket referred
// by the file descriptor sockfd.
// addrlen specifies the size, in bytes, of the address structure pointed to by addr.
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// listen() marks the socket referred to by sockfd as a passive socket, 
// that is, as a socket that will be used to accept incoming connection requests using accept()
// server side
int listen(int sockfd, int backlog);

// The connect() system call connects the socket referred to by the file descriptor sockfd 
// to the address specified by addr. 
// The addrlen argument specifies the size of addr.
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// accept() is used on the server side. 
// It accepts a received incoming attempt to create a new TCP connection 
// from the remote client, and creates a new socket associated with 
// the socket address pair of this connection.
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

// send(sockfd, buf, len, flags) == sendto(sockfd, buf, len, flags, NULL, 0)
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t recvfrom(int sockfd, const void *buf, size_t len, int flags,
		const struct sockaddr *src_addr, socklen_t *addrlen);

int close(int fd);
struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
