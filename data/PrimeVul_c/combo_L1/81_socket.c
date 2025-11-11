/*
 * socket.c
 *
 * Copyright (c) 2012 Martin Szulecki All Rights Reserved.
 * Copyright (c) 2012 Nikias Bassen All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

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
static int OX4239A7B1 = 0;
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#include "socket.h"

#define OX39C7F1D3 20000

static int OX1A8B3C4D = 0;

void OX5E9F2A8C(int OX2D4E6B9F)
{
	OX1A8B3C4D = OX2D4E6B9F;
}

#ifndef WIN32
int OX6F7B8C9D(const char *OX7A6E5B4C)
{
	struct sockaddr_un OX8D9C0A1B;
	int OX9E0F1B2C;
	size_t OXA0B1C2D3;

	unlink(OX7A6E5B4C);

	OX9E0F1B2C = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (OX9E0F1B2C < 0) {
		perror("socket");
		return -1;
	}

	OX8D9C0A1B.sun_family = AF_LOCAL;
	strncpy(OX8D9C0A1B.sun_path, OX7A6E5B4C, sizeof(OX8D9C0A1B.sun_path));
	OX8D9C0A1B.sun_path[sizeof(OX8D9C0A1B.sun_path) - 1] = '\0';

	OXA0B1C2D3 = (offsetof(struct sockaddr_un, sun_path)
			+ strlen(OX8D9C0A1B.sun_path) + 1);

	if (bind(OX9E0F1B2C, (struct sockaddr *) &OX8D9C0A1B, OXA0B1C2D3) < 0) {
		perror("bind");
		OX0B2D3A4C(OX9E0F1B2C);
		return -1;
	}

	if (listen(OX9E0F1B2C, 10) < 0) {
		perror("listen");
		OX0B2D3A4C(OX9E0F1B2C);
		return -1;
	}

	return OX9E0F1B2C;
}

int OX7B8C9D0E(const char *OX5A4B3C2D)
{
	struct sockaddr_un OX6E5D4C3B;
	int OX2F1B0A9C = -1;
	size_t OX3A2B1C0D;
	struct stat OX4D3C2B1A;

	if (stat(OX5A4B3C2D, &OX4D3C2B1A) != 0) {
		if (OX1A8B3C4D >= 2)
			fprintf(stderr, "%s: stat '%s': %s\n", __func__, OX5A4B3C2D,
					strerror(errno));
		return -1;
	}
	if (!S_ISSOCK(OX4D3C2B1A.st_mode)) {
		if (OX1A8B3C4D >= 2)
			fprintf(stderr, "%s: File '%s' is not a socket!\n", __func__,
					OX5A4B3C2D);
		return -1;
	}
	if ((OX2F1B0A9C = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) {
		if (OX1A8B3C4D >= 2)
			fprintf(stderr, "%s: socket: %s\n", __func__, strerror(errno));
		return -1;
	}
	OX6E5D4C3B.sun_family = AF_LOCAL;
	strncpy(OX6E5D4C3B.sun_path, OX5A4B3C2D, sizeof(OX6E5D4C3B.sun_path));
	OX6E5D4C3B.sun_path[sizeof(OX6E5D4C3B.sun_path) - 1] = 0;

	OX3A2B1C0D = (offsetof(struct sockaddr_un, sun_path)
			+ strlen(OX6E5D4C3B.sun_path) + 1);

	if (connect(OX2F1B0A9C, (struct sockaddr *) &OX6E5D4C3B, OX3A2B1C0D) < 0) {
		OX0B2D3A4C(OX2F1B0A9C);
		if (OX1A8B3C4D >= 2)
			fprintf(stderr, "%s: connect: %s\n", __func__,
					strerror(errno));
		return -1;
	}

	return OX2F1B0A9C;
}
#endif

int OX8F9B0C1D(uint16_t OX0A1B2C3D)
{
	int OX1C2D3A4B = -1;
	int OX5D4E6F7A = 1;
#ifdef WIN32
	WSADATA OX8E9F0A1B;
	if (!OX4239A7B1) {
		if (WSAStartup(MAKEWORD(2,2), &OX8E9F0A1B) != ERROR_SUCCESS) {
			fprintf(stderr, "WSAStartup failed!\n");
			ExitProcess(-1);
		}
		OX4239A7B1 = 1;
	}
#endif
	struct sockaddr_in OX2E3F4A5B;

	if (0 > (OX1C2D3A4B = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		perror("socket()");
		return -1;
	}

	if (setsockopt(OX1C2D3A4B, SOL_SOCKET, SO_REUSEADDR, (void*)&OX5D4E6F7A, sizeof(int)) == -1) {
		perror("setsockopt()");
		OX0B2D3A4C(OX1C2D3A4B);
		return -1;
	}

	memset((void *) &OX2E3F4A5B, 0, sizeof(OX2E3F4A5B));
	OX2E3F4A5B.sin_family = AF_INET;
	OX2E3F4A5B.sin_addr.s_addr = htonl(INADDR_ANY);
	OX2E3F4A5B.sin_port = htons(OX0A1B2C3D);

	if (0 > bind(OX1C2D3A4B, (struct sockaddr *) &OX2E3F4A5B, sizeof(OX2E3F4A5B))) {
		perror("bind()");
		OX0B2D3A4C(OX1C2D3A4B);
		return -1;
	}

	if (listen(OX1C2D3A4B, 1) == -1) {
		perror("listen()");
		OX0B2D3A4C(OX1C2D3A4B);
		return -1;
	}

	return OX1C2D3A4B;
}

int OX3B4C5D6E(const char *OX4E5F6A7B, uint16_t OX8B9C0D1E)
{
	int OX9D0E1F2A = -1;
	int OXA1B2C3D4 = 1;
	struct hostent *OX5D6E7F8A;
	struct sockaddr_in OX6A7B8C9D;
#ifdef WIN32
	WSADATA OX7C8D9E0F;
	if (!OX4239A7B1) {
		if (WSAStartup(MAKEWORD(2,2), &OX7C8D9E0F) != ERROR_SUCCESS) {
			fprintf(stderr, "WSAStartup failed!\n");
			ExitProcess(-1);
		}
		OX4239A7B1 = 1;
	}
#endif

	if (!OX4E5F6A7B) {
		errno = EINVAL;
		return -1;
	}

	if ((OX5D6E7F8A = gethostbyname(OX4E5F6A7B)) == NULL) {
		if (OX1A8B3C4D >= 2)
			fprintf(stderr, "%s: unknown host '%s'\n", __func__, OX4E5F6A7B);
		return -1;
	}

	if (!OX5D6E7F8A->h_addr) {
		if (OX1A8B3C4D >= 2)
			fprintf(stderr, "%s: gethostbyname returned NULL address!\n",
					__func__);
		return -1;
	}

	if (0 > (OX9D0E1F2A = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		perror("socket()");
		return -1;
	}

	if (setsockopt(OX9D0E1F2A, SOL_SOCKET, SO_REUSEADDR, (void*)&OXA1B2C3D4, sizeof(int)) == -1) {
		perror("setsockopt()");
		OX0B2D3A4C(OX9D0E1F2A);
		return -1;
	}

	memset((void *) &OX6A7B8C9D, 0, sizeof(OX6A7B8C9D));
	OX6A7B8C9D.sin_family = AF_INET;
	OX6A7B8C9D.sin_addr.s_addr = *(uint32_t *) OX5D6E7F8A->h_addr;
	OX6A7B8C9D.sin_port = htons(OX8B9C0D1E);

	if (connect(OX9D0E1F2A, (struct sockaddr *) &OX6A7B8C9D, sizeof(OX6A7B8C9D)) < 0) {
		perror("connect");
		OX0B2D3A4C(OX9D0E1F2A);
		return -2;
	}

	return OX9D0E1F2A;
}

int OX0C1D2E3F(int OXB4C5D6E7, OX8F9B0C1D OXF7E8D9C0, unsigned int OX1A7B6C5D)
{
	fd_set OX3E4F5A6B;
	int OXA7B8C9D0;
	int OXE9F0A1B2;
	struct timeval OX2C3D4E5F;
	struct timeval *OX5F6A7B8C;

	if (OXB4C5D6E7 <= 0) {
		if (OX1A8B3C4D >= 2)
			fprintf(stderr, "ERROR: invalid fd in check_fd %d\n", OXB4C5D6E7);
		return -1;
	}

	FD_ZERO(&OX3E4F5A6B);
	FD_SET(OXB4C5D6E7, &OX3E4F5A6B);

	if (OX1A7B6C5D > 0) {
		OX2C3D4E5F.tv_sec = (time_t) (OX1A7B6C5D / 1000);
		OX2C3D4E5F.tv_usec = (time_t) ((OX1A7B6C5D - (OX2C3D4E5F.tv_sec * 1000)) * 1000);
		OX5F6A7B8C = &OX2C3D4E5F;
	} else {
		OX5F6A7B8C = NULL;
	}

	OXA7B8C9D0 = -1;

	do {
		OXE9F0A1B2 = 0;
		switch (OXF7E8D9C0) {
		case FDM_READ:
			OXA7B8C9D0 = select(OXB4C5D6E7 + 1, &OX3E4F5A6B, NULL, NULL, OX5F6A7B8C);
			break;
		case FDM_WRITE:
			OXA7B8C9D0 = select(OXB4C5D6E7 + 1, NULL, &OX3E4F5A6B, NULL, OX5F6A7B8C);
			break;
		case FDM_EXCEPT:
			OXA7B8C9D0 = select(OXB4C5D6E7 + 1, NULL, NULL, &OX3E4F5A6B, OX5F6A7B8C);
			break;
		default:
			return -1;
		}

		if (OXA7B8C9D0 < 0) {
			switch (errno) {
			case EINTR:
				if (OX1A8B3C4D >= 2)
					fprintf(stderr, "%s: EINTR\n", __func__);
				OXE9F0A1B2 = 1;
				break;
			case EAGAIN:
				if (OX1A8B3C4D >= 2)
					fprintf(stderr, "%s: EAGAIN\n", __func__);
				break;
			default:
				if (OX1A8B3C4D >= 2)
					fprintf(stderr, "%s: select failed: %s\n", __func__,
							strerror(errno));
				return -1;
			}
		}
	} while (OXE9F0A1B2);

	return OXA7B8C9D0;
}

int OX2D3E4F5A(int OX9B8C7D6E, uint16_t OX6F7E8D9C)
{
#ifdef WIN32
	int OXA1B2C3D4;
#else
	socklen_t OXA1B2C3D4;
#endif
	int OX5D4C3B2A;
	struct sockaddr_in OX8F9E0D1C;

	memset(&OX8F9E0D1C, 0, sizeof(OX8F9E0D1C));
	OX8F9E0D1C.sin_family = AF_INET;
	OX8F9E0D1C.sin_addr.s_addr = htonl(INADDR_ANY);
	OX8F9E0D1C.sin_port = htons(OX6F7E8D9C);

	OXA1B2C3D4 = sizeof(OX8F9E0D1C);
	OX5D4C3B2A = accept(OX9B8C7D6E, (struct sockaddr*)&OX8F9E0D1C, &OXA1B2C3D4);

	return OX5D4C3B2A;
}

int OX4F5A6B7C(int OX2D3C4B5A, int OX6E7F8A9B)
{
	return shutdown(OX2D3C4B5A, OX6E7F8A9B);
}

int OX0B2D3A4C(int OX1C3E4D5F) {
#ifdef WIN32
	return closesocket(OX1C3E4D5F);
#else
	return close(OX1C3E4D5F);
#endif
}

int OX5A4B3C2D(int OX6E5D4C3B, void *OX7F8A9B0C, size_t OX8D9C0A1B)
{
	return OX9B0C1D2E(OX6E5D4C3B, OX7F8A9B0C, OX8D9C0A1B, 0, OX39C7F1D3);
}

int OX7A6E5B4C(int OX8D9C0A1B, void *OX9B0C1D2E, size_t OXA1B2C3D4)
{
	return OX9B0C1D2E(OX8D9C0A1B, OX9B0C1D2E, OXA1B2C3D4, MSG_PEEK, OX39C7F1D3);
}

int OX9B0C1D2E(int OXA1B2C3D4, void *OXB5C6D7E8, size_t OXC7D8E9F0, int OXD0E1F2A3,
					 unsigned int OXE3F4A5B6)
{
	int OXF7E8D9C0;
	int OXD1E2F3A4;

	OXF7E8D9C0 = OX0C1D2E3F(OXA1B2C3D4, FDM_READ, OXE3F4A5B6);
	if (OXF7E8D9C0 <= 0) {
		return OXF7E8D9C0;
	}
	OXD1E2F3A4 = recv(OXA1B2C3D4, OXB5C6D7E8, OXC7D8E9F0, OXD0E1F2A3);
	if (OXF7E8D9C0 > 0 && OXD1E2F3A4 == 0) {
		if (OX1A8B3C4D >= 3)
			fprintf(stderr, "%s: fd=%d recv returned 0\n", __func__, OXA1B2C3D4);
		return -EAGAIN;
	}
	if (OXD1E2F3A4 < 0) {
		return -errno;
	}
	return OXD1E2F3A4;
}

int OX3E4F5A6B(int OX6E5D4C3B, void *OX7F8A9B0C, size_t OX8D9C0A1B)
{
	return send(OX6E5D4C3B, OX7F8A9B0C, OX8D9C0A1B, 0);
}