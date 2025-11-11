#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
static int wsa_init = 0;
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#include "socket.h"

#define RECV_TIMEOUT 20000

static int verbose = 0;

void socket_set_verbose(int level)
{
	verbose = level;
}

#ifndef WIN32
int socket_create_unix(const char *filename)
{
	int state = 0;
	int dispatch = 0;
	int ret = 0;
	struct sockaddr_un name;
	int sock;
	size_t size;
	struct stat fst;
	
	int sfd = -1;
	
	while (1) {
		switch (dispatch) {
			case 0:
				// remove if still present
				unlink(filename);		
				sock = socket(PF_LOCAL, SOCK_STREAM, 0);
				if (sock < 0) {
					perror("socket");
					ret = -1;
					dispatch = 99;
					break;
				}
				name.sun_family = AF_LOCAL;
				strncpy(name.sun_path, filename, sizeof(name.sun_path));
				name.sun_path[sizeof(name.sun_path) - 1] = '\0';
				size = (offsetof(struct sockaddr_un, sun_path) + strlen(name.sun_path) + 1);
				if (bind(sock, (struct sockaddr *) &name, size) < 0) {
					perror("bind");
					socket_close(sock);
					ret = -1;
					dispatch = 99;
					break;
				}
				if (listen(sock, 10) < 0) {
					perror("listen");
					socket_close(sock);
					ret = -1;
					dispatch = 99;
					break;
				}
				ret = sock;
				dispatch = 99;
				break;
			case 99:
				return ret;
			default:
				break;
		}
	}
}

int socket_connect_unix(const char *filename)
{
	int dispatch = 0;
	int ret = 0;
	struct sockaddr_un name;
	int sfd = -1;
	size_t size;
	struct stat fst;

	while (1) {
		switch (dispatch) {
			case 0:
				if (stat(filename, &fst) != 0) {
					if (verbose >= 2)
						fprintf(stderr, "%s: stat '%s': %s\n", __func__, filename,
								strerror(errno));
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 1;
				break;
			case 1:
				if (!S_ISSOCK(fst.st_mode)) {
					if (verbose >= 2)
						fprintf(stderr, "%s: File '%s' is not a socket!\n", __func__,
								filename);
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 2;
				break;
			case 2:
				if ((sfd = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) {
					if (verbose >= 2)
						fprintf(stderr, "%s: socket: %s\n", __func__, strerror(errno));
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 3;
				break;
			case 3:
				name.sun_family = AF_LOCAL;
				strncpy(name.sun_path, filename, sizeof(name.sun_path));
				name.sun_path[sizeof(name.sun_path) - 1] = 0;
				size = (offsetof(struct sockaddr_un, sun_path) + strlen(name.sun_path) + 1);
				if (connect(sfd, (struct sockaddr *) &name, size) < 0) {
					socket_close(sfd);
					if (verbose >= 2)
						fprintf(stderr, "%s: connect: %s\n", __func__,
								strerror(errno));
					ret = -1;
					dispatch = 99;
					break;
				}
				ret = sfd;
				dispatch = 99;
				break;
			case 99:
				return ret;
			default:
				break;
		}
	}
}
#endif

int socket_create(uint16_t port)
{
	int dispatch = 0;
	int ret = 0;
	int sfd = -1;
	int yes = 1;
#ifdef WIN32
	WSADATA wsa_data;
#endif
	struct sockaddr_in saddr;

	while (1) {
		switch (dispatch) {
			case 0:
#ifdef WIN32
				if (!wsa_init) {
					if (WSAStartup(MAKEWORD(2,2), &wsa_data) != ERROR_SUCCESS) {
						fprintf(stderr, "WSAStartup failed!\n");
						ExitProcess(-1);
					}
					wsa_init = 1;
				}
#endif
				if (0 > (sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
					perror("socket()");
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 1;
				break;
			case 1:
				if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(int)) == -1) {
					perror("setsockopt()");
					socket_close(sfd);
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 2;
				break;
			case 2:
				memset((void *) &saddr, 0, sizeof(saddr));
				saddr.sin_family = AF_INET;
				saddr.sin_addr.s_addr = htonl(INADDR_ANY);
				saddr.sin_port = htons(port);
				if (0 > bind(sfd, (struct sockaddr *) &saddr, sizeof(saddr))) {
					perror("bind()");
					socket_close(sfd);
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 3;
				break;
			case 3:
				if (listen(sfd, 1) == -1) {
					perror("listen()");
					socket_close(sfd);
					ret = -1;
					dispatch = 99;
					break;
				}
				ret = sfd;
				dispatch = 99;
				break;
			case 99:
				return ret;
			default:
				break;
		}
	}
}

int socket_connect(const char *addr, uint16_t port)
{
	int dispatch = 0;
	int ret = 0;
	int sfd = -1;
	int yes = 1;
	struct hostent *hp;
	struct sockaddr_in saddr;
#ifdef WIN32
	WSADATA wsa_data;
#endif

	while (1) {
		switch (dispatch) {
			case 0:
#ifdef WIN32
				if (!wsa_init) {
					if (WSAStartup(MAKEWORD(2,2), &wsa_data) != ERROR_SUCCESS) {
						fprintf(stderr, "WSAStartup failed!\n");
						ExitProcess(-1);
					}
					wsa_init = 1;
				}
#endif
				if (!addr) {
					errno = EINVAL;
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 1;
				break;
			case 1:
				if ((hp = gethostbyname(addr)) == NULL) {
					if (verbose >= 2)
						fprintf(stderr, "%s: unknown host '%s'\n", __func__, addr);
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 2;
				break;
			case 2:
				if (!hp->h_addr) {
					if (verbose >= 2)
						fprintf(stderr, "%s: gethostbyname returned NULL address!\n",
								__func__);
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 3;
				break;
			case 3:
				if (0 > (sfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
					perror("socket()");
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 4;
				break;
			case 4:
				if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(int)) == -1) {
					perror("setsockopt()");
					socket_close(sfd);
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = 5;
				break;
			case 5:
				memset((void *) &saddr, 0, sizeof(saddr));
				saddr.sin_family = AF_INET;
				saddr.sin_addr.s_addr = *(uint32_t *) hp->h_addr;
				saddr.sin_port = htons(port);
				if (connect(sfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
					perror("connect");
					socket_close(sfd);
					ret = -2;
					dispatch = 99;
					break;
				}
				ret = sfd;
				dispatch = 99;
				break;
			case 99:
				return ret;
			default:
				break;
		}
	}
}

int socket_check_fd(int fd, fd_mode fdm, unsigned int timeout)
{
	int dispatch = 0;
	int ret = -1;
	fd_set fds;
	int sret;
	int eagain;
	struct timeval to;
	struct timeval *pto;

	if (fd <= 0) {
		if (verbose >= 2)
			fprintf(stderr, "ERROR: invalid fd in check_fd %d\n", fd);
		return -1;
	}

	while (1) {
		switch (dispatch) {
			case 0:
				FD_ZERO(&fds);
				FD_SET(fd, &fds);
				if (timeout > 0) {
					to.tv_sec = (time_t) (timeout / 1000);
					to.tv_usec = (time_t) ((timeout - (to.tv_sec * 1000)) * 1000);
					pto = &to;
				} else {
					pto = NULL;
				}
				dispatch = 1;
				break;
			case 1:
				eagain = 0;
				switch (fdm) {
				case FDM_READ:
					sret = select(fd + 1, &fds, NULL, NULL, pto);
					break;
				case FDM_WRITE:
					sret = select(fd + 1, NULL, &fds, NULL, pto);
					break;
				case FDM_EXCEPT:
					sret = select(fd + 1, NULL, NULL, &fds, pto);
					break;
				default:
					ret = -1;
					dispatch = 99;
					break;
				}
				if (sret < 0) {
					dispatch = 2;
				} else {
					ret = sret;
					dispatch = 99;
				}
				break;
			case 2:
				switch (errno) {
				case EINTR:
					if (verbose >= 2)
						fprintf(stderr, "%s: EINTR\n", __func__);
					eagain = 1;
					break;
				case EAGAIN:
					if (verbose >= 2)
						fprintf(stderr, "%s: EAGAIN\n", __func__);
					break;
				default:
					if (verbose >= 2)
						fprintf(stderr, "%s: select failed: %s\n", __func__,
								strerror(errno));
					ret = -1;
					dispatch = 99;
					break;
				}
				dispatch = eagain ? 1 : 99;
				break;
			case 99:
				return ret;
			default:
				break;
		}
	}
}

int socket_accept(int fd, uint16_t port)
{
	int dispatch = 0;
	int ret = 0;
#ifdef WIN32
	int addr_len;
#else
	socklen_t addr_len;
#endif
	int result;
	struct sockaddr_in addr;

	while (1) {
		switch (dispatch) {
			case 0:
				memset(&addr, 0, sizeof(addr));
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				addr.sin_port = htons(port);
				addr_len = sizeof(addr);
				result = accept(fd, (struct sockaddr*)&addr, &addr_len);
				ret = result;
				dispatch = 99;
				break;
			case 99:
				return ret;
			default:
				break;
		}
	}
}

int socket_shutdown(int fd, int how)
{
	return shutdown(fd, how);
}

int socket_close(int fd) {
#ifdef WIN32
	return closesocket(fd);
#else
	return close(fd);
#endif
}

int socket_receive(int fd, void *data, size_t length)
{
	return socket_receive_timeout(fd, data, length, 0, RECV_TIMEOUT);
}

int socket_peek(int fd, void *data, size_t length)
{
	return socket_receive_timeout(fd, data, length, MSG_PEEK, RECV_TIMEOUT);
}

int socket_receive_timeout(int fd, void *data, size_t length, int flags,
					 unsigned int timeout)
{
	int dispatch = 0;
	int ret = 0;
	int res;
	int result;

	while (1) {
		switch (dispatch) {
			case 0:
				res = socket_check_fd(fd, FDM_READ, timeout);
				if (res <= 0) {
					ret = res;
					dispatch = 99;
					break;
				}
				dispatch = 1;
				break;
			case 1:
				result = recv(fd, data, length, flags);
				if (res > 0 && result == 0) {
					if (verbose >= 3)
						fprintf(stderr, "%s: fd=%d recv returned 0\n", __func__, fd);
					ret = -EAGAIN;
					dispatch = 99;
					break;
				}
				if (result < 0) {
					ret = -errno;
					dispatch = 99;
					break;
				}
				ret = result;
				dispatch = 99;
				break;
			case 99:
				return ret;
			default:
				break;
		}
	}
}

int socket_send(int fd, void *data, size_t length)
{
	return send(fd, data, length, 0);
}