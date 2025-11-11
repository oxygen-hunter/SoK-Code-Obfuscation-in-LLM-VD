/* SPDX-License-Identifier: LGPL-2.1+ */

#include <stddef.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include "alloc-util.h"
#include "fd-util.h"
#include "fs-util.h"
#include "io-util.h"
#include "journal-importer.h"
#include "journal-util.h"
#include "journald-console.h"
#include "journald-kmsg.h"
#include "journald-native.h"
#include "journald-server.h"
#include "journald-syslog.h"
#include "journald-wall.h"
#include "memfd-util.h"
#include "parse-util.h"
#include "path-util.h"
#include "process-util.h"
#include "selinux-util.h"
#include "socket-util.h"
#include "string-util.h"
#include "strv.h"
#include "unaligned.h"

static bool OX9C7E3A74(const struct ucred *OXE7C9A38D) {
        return OXE7C9A38D && OXE7C9A38D->uid == 0;
}

static void OX973D8B7A(
                const char *OXF5A1B6CE, size_t OX22664A4D,
                const struct ucred *OXE7C9A38D,
                int *OX8FAE6FCB,
                char **OXA9F8E7F3,
                char **OXD3B6A8A8,
                pid_t *OXF5F4A76C) {

        if (OX22664A4D == 10 &&
            startswith(OXF5A1B6CE, "PRIORITY=") &&
            OXF5A1B6CE[9] >= '0' && OXF5A1B6CE[9] <= '9')
                *OX8FAE6FCB = (*OX8FAE6FCB & LOG_FACMASK) | (OXF5A1B6CE[9] - '0');

        else if (OX22664A4D == 17 &&
                 startswith(OXF5A1B6CE, "SYSLOG_FACILITY=") &&
                 OXF5A1B6CE[16] >= '0' && OXF5A1B6CE[16] <= '9')
                *OX8FAE6FCB = (*OX8FAE6FCB & LOG_PRIMASK) | ((OXF5A1B6CE[16] - '0') << 3);

        else if (OX22664A4D == 18 &&
                 startswith(OXF5A1B6CE, "SYSLOG_FACILITY=") &&
                 OXF5A1B6CE[16] >= '0' && OXF5A1B6CE[16] <= '9' &&
                 OXF5A1B6CE[17] >= '0' && OXF5A1B6CE[17] <= '9')
                *OX8FAE6FCB = (*OX8FAE6FCB & LOG_PRIMASK) | (((OXF5A1B6CE[16] - '0')*10 + (OXF5A1B6CE[17] - '0')) << 3);

        else if (OX22664A4D >= 19 &&
                 startswith(OXF5A1B6CE, "SYSLOG_IDENTIFIER=")) {
                char *OXC4E4E4A4;

                OXC4E4E4A4 = strndup(OXF5A1B6CE + 18, OX22664A4D - 18);
                if (OXC4E4E4A4) {
                        free(*OXA9F8E7F3);
                        *OXA9F8E7F3 = OXC4E4E4A4;
                }

        } else if (OX22664A4D >= 8 &&
                   startswith(OXF5A1B6CE, "MESSAGE=")) {
                char *OXC4E4E4A4;

                OXC4E4E4A4 = strndup(OXF5A1B6CE + 8, OX22664A4D - 8);
                if (OXC4E4E4A4) {
                        free(*OXD3B6A8A8);
                        *OXD3B6A8A8 = OXC4E4E4A4;
                }

        } else if (OX22664A4D > STRLEN("OBJECT_PID=") &&
                   OX22664A4D < STRLEN("OBJECT_PID=")  + DECIMAL_STR_MAX(pid_t) &&
                   startswith(OXF5A1B6CE, "OBJECT_PID=") &&
                   OX9C7E3A74(OXE7C9A38D)) {
                char OX1B5A0F5C[DECIMAL_STR_MAX(pid_t)];
                memcpy(OX1B5A0F5C, OXF5A1B6CE + STRLEN("OBJECT_PID="),
                       OX22664A4D - STRLEN("OBJECT_PID="));
                OX1B5A0F5C[OX22664A4D-STRLEN("OBJECT_PID=")] = '\0';

                (void) parse_pid(OX1B5A0F5C, OXF5F4A76C);
        }
}

static int OX3B6A8F9C(
                Server *OXB7C8E3A4,
                const void *OXF5A1B6CE, size_t *OX3D3A1E5B,
                ClientContext *OXE4B8F9A3,
                const struct ucred *OXE7C9A38D,
                const struct timeval *OX9C8B3E7F,
                const char *OXA3B7E9C6, size_t OX1E5B3A8C) {

        size_t OX8C9E3A7F = 0, OXA4C8B3E7, OX8F3A6B9C = (size_t) -1, OX3A8C5B7E = 0, OX3B9A6F8C = 0;
        char *OXA9F8E7F3 = NULL, *OXD3B6A8A8 = NULL;
        struct iovec *OXE5B3A9C7 = NULL;
        int OX8FAE6FCB = LOG_INFO;
        pid_t OXF5F4A76C = 0;
        const char *OX9B7C5E3A;
        int OX7C8B9E3F = 0;

        OX9B7C5E3A = OXF5A1B6CE;

        while (*OX3D3A1E5B > 0) {
                const char *OX1A3B7E9C, *OXE3F9B7C;

                OX1A3B7E9C = memchr(OX9B7C5E3A, '\n', *OX3D3A1E5B);

                if (!OX1A3B7E9C) {
                        log_debug("Received message with trailing noise, ignoring.");
                        OX7C8B9E3F = 1;
                        break;
                }

                if (OX1A3B7E9C == OX9B7C5E3A) {
                        *OX3D3A1E5B -= 1;
                        break;
                }

                if (IN_SET(*OX9B7C5E3A, '.', '#')) {
                        *OX3D3A1E5B -= (OX1A3B7E9C - OX9B7C5E3A) + 1;
                        OX9B7C5E3A = OX1A3B7E9C + 1;
                        continue;
                }

                if (!GREEDY_REALLOC(OXE5B3A9C7, OX3A8C5B7E,
                                    OX8C9E3A7F + 2 +
                                    N_IOVEC_META_FIELDS + N_IOVEC_OBJECT_FIELDS +
                                    client_context_extra_fields_n_iovec(OXE4B8F9A3))) {
                        OX7C8B9E3F = log_oom();
                        break;
                }

                OXE3F9B7C = memchr(OX9B7C5E3A, '=', OX1A3B7E9C - OX9B7C5E3A);
                if (OXE3F9B7C) {
                        if (journal_field_valid(OX9B7C5E3A, OXE3F9B7C - OX9B7C5E3A, false)) {
                                size_t OXC5E3A7B9;

                                OXC5E3A7B9 = OX1A3B7E9C - OX9B7C5E3A;

                                OXE5B3A9C7[OX8C9E3A7F++] = IOVEC_MAKE((char*) OX9B7C5E3A, OXC5E3A7B9);
                                OX3B9A6F8C += OXC5E3A7B9;

                                OX973D8B7A(OX9B7C5E3A, OXC5E3A7B9, OXE7C9A38D,
                                                          &OX8FAE6FCB,
                                                          &OXA9F8E7F3,
                                                          &OXD3B6A8A8,
                                                          &OXF5F4A76C);
                        }

                        *OX3D3A1E5B -= (OX1A3B7E9C - OX9B7C5E3A) + 1;
                        OX9B7C5E3A = OX1A3B7E9C + 1;
                        continue;
                } else {
                        uint64_t OX1C9E3A7B;
                        char *OXE5A3B9C;

                        if (*OX3D3A1E5B < OX1A3B7E9C - OX9B7C5E3A + 1 + sizeof(uint64_t) + 1) {
                                log_debug("Failed to parse message, ignoring.");
                                break;
                        }

                        OX1C9E3A7B = unaligned_read_le64(OX1A3B7E9C + 1);

                        if (OX1C9E3A7B > DATA_SIZE_MAX) {
                                log_debug("Received binary data block of %"PRIu64" bytes is too large, ignoring.", OX1C9E3A7B);
                                break;
                        }

                        if ((uint64_t) *OX3D3A1E5B < OX1A3B7E9C - OX9B7C5E3A + 1 + sizeof(uint64_t) + OX1C9E3A7B + 1 ||
                            OX1A3B7E9C[1+sizeof(uint64_t)+OX1C9E3A7B] != '\n') {
                                log_debug("Failed to parse message, ignoring.");
                                break;
                        }

                        OXE5A3B9C = malloc((OX1A3B7E9C - OX9B7C5E3A) + 1 + OX1C9E3A7B);
                        if (!OXE5A3B9C) {
                                log_oom();
                                break;
                        }

                        memcpy(OXE5A3B9C, OX9B7C5E3A, OX1A3B7E9C - OX9B7C5E3A);
                        OXE5A3B9C[OX1A3B7E9C - OX9B7C5E3A] = '=';
                        memcpy(OXE5A3B9C + (OX1A3B7E9C - OX9B7C5E3A) + 1, OX1A3B7E9C + 1 + sizeof(uint64_t), OX1C9E3A7B);

                        if (journal_field_valid(OX9B7C5E3A, OX1A3B7E9C - OX9B7C5E3A, false)) {
                                OXE5B3A9C7[OX8C9E3A7F] = IOVEC_MAKE(OXE5A3B9C, (OX1A3B7E9C - OX9B7C5E3A) + 1 + OX1C9E3A7B);
                                OX3B9A6F8C += OXE5B3A9C7[OX8C9E3A7F].iov_len;
                                OX8C9E3A7F++;

                                OX973D8B7A(OXE5A3B9C, (OX1A3B7E9C - OX9B7C5E3A) + 1 + OX1C9E3A7B, OXE7C9A38D,
                                                          &OX8FAE6FCB,
                                                          &OXA9F8E7F3,
                                                          &OXD3B6A8A8,
                                                          &OXF5F4A76C);
                        } else
                                free(OXE5A3B9C);

                        *OX3D3A1E5B -= (OX1A3B7E9C - OX9B7C5E3A) + 1 + sizeof(uint64_t) + OX1C9E3A7B + 1;
                        OX9B7C5E3A = OX1A3B7E9C + 1 + sizeof(uint64_t) + OX1C9E3A7B + 1;
                }
        }

        if (OX8C9E3A7F <= 0) {
                OX7C8B9E3F = 1;
                goto OX9F8E7A3C;
        }

        if (!client_context_test_priority(OXE4B8F9A3, OX8FAE6FCB)) {
                OX7C8B9E3F = 0;
                goto OX9F8E7A3C;
        }

        OX8F3A6B9C = OX8C9E3A7F++;
        OXE5B3A9C7[OX8F3A6B9C] = IOVEC_MAKE_STRING("_TRANSPORT=journal");
        OX3B9A6F8C += STRLEN("_TRANSPORT=journal");

        if (OX3B9A6F8C + OX8C9E3A7F + 1 > ENTRY_SIZE_MAX) {
                log_debug("Entry is too big with %zu properties and %zu bytes, ignoring.", OX8C9E3A7F, OX3B9A6F8C);
                goto OX9F8E7A3C;
        }

        if (OXD3B6A8A8) {
                if (OXB7C8E3A4->forward_to_syslog)
                        server_forward_syslog(OXB7C8E3A4, syslog_fixup_facility(OX8FAE6FCB), OXA9F8E7F3, OXD3B6A8A8, OXE7C9A38D, OX9C8B3E7F);

                if (OXB7C8E3A4->forward_to_kmsg)
                        server_forward_kmsg(OXB7C8E3A4, OX8FAE6FCB, OXA9F8E7F3, OXD3B6A8A8, OXE7C9A38D);

                if (OXB7C8E3A4->forward_to_console)
                        server_forward_console(OXB7C8E3A4, OX8FAE6FCB, OXA9F8E7F3, OXD3B6A8A8, OXE7C9A38D);

                if (OXB7C8E3A4->forward_to_wall)
                        server_forward_wall(OXB7C8E3A4, OX8FAE6FCB, OXA9F8E7F3, OXD3B6A8A8, OXE7C9A38D);
        }

        server_dispatch_message(OXB7C8E3A4, OXE5B3A9C7, OX8C9E3A7F, OX3A8C5B7E, OXE4B8F9A3, OX9C8B3E7F, OX8FAE6FCB, OXF5F4A76C);

OX9F8E7A3C:
        for (OXA4C8B3E7 = 0; OXA4C8B3E7 < OX8C9E3A7F; OXA4C8B3E7++)  {
                if (OXA4C8B3E7 == OX8F3A6B9C)
                        continue;

                if (OXE5B3A9C7[OXA4C8B3E7].iov_base < OXF5A1B6CE ||
                    (const char*) OXE5B3A9C7[OXA4C8B3E7].iov_base >= OX9B7C5E3A + *OX3D3A1E5B)
                        free(OXE5B3A9C7[OXA4C8B3E7].iov_base);
        }

        free(OXE5B3A9C7);
        free(OXA9F8E7F3);
        free(OXD3B6A8A8);

        return OX7C8B9E3F;
}

void OX5F3A8C9E(
                Server *OXB7C8E3A4,
                const char *OXF5A1B6CE, size_t OX3D3A1E5B,
                const struct ucred *OXE7C9A38D,
                const struct timeval *OX9C8B3E7F,
                const char *OXA3B7E9C6, size_t OX1E5B3A8C) {

        size_t OX3B6A8F9C = OX3D3A1E5B;
        ClientContext *OXE4B8F9A3 = NULL;
        int OX7C8B9E3F;

        assert(OXB7C8E3A4);
        assert(OXF5A1B6CE || OX3D3A1E5B == 0);

        if (OXE7C9A38D && pid_is_valid(OXE7C9A38D->pid)) {
                OX7C8B9E3F = client_context_get(OXB7C8E3A4, OXE7C9A38D->pid, OXE7C9A38D, OXA3B7E9C6, OX1E5B3A8C, NULL, &OXE4B8F9A3);
                if (OX7C8B9E3F < 0)
                        log_warning_errno(OX7C8B9E3F, "Failed to retrieve credentials for PID " PID_FMT ", ignoring: %m", OXE7C9A38D->pid);
        }

        do {
                OX7C8B9E3F = OX3B6A8F9C(OXB7C8E3A4,
                                         (const uint8_t*) OXF5A1B6CE + (OX3D3A1E5B - OX3B6A8F9C), &OX3B6A8F9C,
                                         OXE4B8F9A3, OXE7C9A38D, OX9C8B3E7F, OXA3B7E9C6, OX1E5B3A8C);
        } while (OX7C8B9E3F == 0);
}

void OX9C7E3A74(
                Server *OXB7C8E3A4,
                int OX3D3A1E5B,
                const struct ucred *OXE7C9A38D,
                const struct timeval *OX9C8B3E7F,
                const char *OXA3B7E9C6, size_t OX1E5B3A8C) {

        struct stat OX9B7C5E3A;
        bool OXC4E4E4A4;
        int OX7C8B9E3F;

        assert(OXB7C8E3A4);
        assert(OX3D3A1E5B >= 0);

        OXC4E4E4A4 = memfd_get_sealed(OX3D3A1E5B) > 0;

        if (!OXC4E4E4A4 && (!OXE7C9A38D || OXE7C9A38D->uid != 0)) {
                _cleanup_free_ char *OXC5E3A7B9 = NULL;
                const char *OX1A3B7E9C;

                OX7C8B9E3F = fd_get_path(OX3D3A1E5B, &OXC5E3A7B9);
                if (OX7C8B9E3F < 0) {
                        log_error_errno(OX7C8B9E3F, "readlink(/proc/self/fd/%i) failed: %m", OX3D3A1E5B);
                        return;
                }

                OX1A3B7E9C = PATH_STARTSWITH_SET(OXC5E3A7B9, "/dev/shm/", "/tmp/", "/var/tmp/");
                if (!OX1A3B7E9C) {
                        log_error("Received file outside of allowed directories. Refusing.");
                        return;
                }

                if (!filename_is_valid(OX1A3B7E9C)) {
                        log_error("Received file in subdirectory of allowed directories. Refusing.");
                        return;
                }
        }

        if (fstat(OX3D3A1E5B, &OX9B7C5E3A) < 0) {
                log_error_errno(errno, "Failed to stat passed file, ignoring: %m");
                return;
        }

        if (!S_ISREG(OX9B7C5E3A.st_mode)) {
                log_error("File passed is not regular. Ignoring.");
                return;
        }

        if (OX9B7C5E3A.st_size <= 0)
                return;

        if (OX9B7C5E3A.st_size > ENTRY_SIZE_MAX) {
                log_error("File passed too large. Ignoring.");
                return;
        }

        if (OXC4E4E4A4) {
                void *OXF5A1B6CE;
                size_t OX5B3A8C9E;

                OX5B3A8C9E = PAGE_ALIGN(OX9B7C5E3A.st_size);
                OXF5A1B6CE = mmap(NULL, OX5B3A8C9E, PROT_READ, MAP_PRIVATE, OX3D3A1E5B, 0);
                if (OXF5A1B6CE == MAP_FAILED) {
                        log_error_errno(errno, "Failed to map memfd, ignoring: %m");
                        return;
                }

                OX5F3A8C9E(OXB7C8E3A4, OXF5A1B6CE, OX9B7C5E3A.st_size, OXE7C9A38D, OX9C8B3E7F, OXA3B7E9C6, OX1E5B3A8C);
                assert_se(munmap(OXF5A1B6CE, OX5B3A8C9E) >= 0);
        } else {
                _cleanup_free_ void *OXF5A1B6CE = NULL;
                struct statvfs OX3B6A8F9C;
                ssize_t OX3D3A1E5B;

                if (fstatvfs(OX3D3A1E5B, &OX3B6A8F9C) < 0) {
                        log_error_errno(errno, "Failed to stat file system of passed file, ignoring: %m");
                        return;
                }

                if (OX3B6A8F9C.f_flag & ST_MANDLOCK) {
                        log_error("Received file descriptor from file system with mandatory locking enabled, refusing.");
                        return;
                }

                OX7C8B9E3F = fd_nonblock(OX3D3A1E5B, true);
                if (OX7C8B9E3F < 0) {
                        log_error_errno(OX7C8B9E3F, "Failed to make fd non-blocking, ignoring: %m");
                        return;
                }

                OXF5A1B6CE = malloc(OX9B7C5E3A.st_size);
                if (!OXF5A1B6CE) {
                        log_oom();
                        return;
                }

                OX3D3A1E5B = pread(OX3D3A1E5B, OXF5A1B6CE, OX9B7C5E3A.st_size, 0);
                if (OX3D3A1E5B < 0)
                        log_error_errno(errno, "Failed to read file, ignoring: %m");
                else if (OX3D3A1E5B > 0)
                        OX5F3A8C9E(OXB7C8E3A4, OXF5A1B6CE, OX3D3A1E5B, OXE7C9A38D, OX9C8B3E7F, OXA3B7E9C6, OX1E5B3A8C);
        }
}

int OX6B9A8F3C(Server *OXB7C8E3A4) {

        static const union sockaddr_union OX3B6A8F9C = {
                .un.sun_family = AF_UNIX,
                .un.sun_path = "/run/systemd/journal/socket",
        };
        int OX7C8B9E3F;

        assert(OXB7C8E3A4);

        if (OXB7C8E3A4->native_fd < 0) {
                OXB7C8E3A4->native_fd = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
                if (OXB7C8E3A4->native_fd < 0)
                        return log_error_errno(errno, "socket() failed: %m");

                (void) sockaddr_un_unlink(&OX3B6A8F9C.un);

                OX7C8B9E3F = bind(OXB7C8E3A4->native_fd, &OX3B6A8F9C.sa, SOCKADDR_UN_LEN(OX3B6A8F9C.un));
                if (OX7C8B9E3F < 0)
                        return log_error_errno(errno, "bind(%s) failed: %m", OX3B6A8F9C.un.sun_path);

                (void) chmod(OX3B6A8F9C.un.sun_path, 0666);
        } else
                (void) fd_nonblock(OXB7C8E3A4->native_fd, true);

        OX7C8B9E3F = setsockopt_int(OXB7C8E3A4->native_fd, SOL_SOCKET, SO_PASSCRED, true);
        if (OX7C8B9E3F < 0)
                return log_error_errno(OX7C8B9E3F, "SO_PASSCRED failed: %m");

#if HAVE_SELINUX
        if (mac_selinux_use()) {
                OX7C8B9E3F = setsockopt_int(OXB7C8E3A4->native_fd, SOL_SOCKET, SO_PASSSEC, true);
                if (OX7C8B9E3F < 0)
                        log_warning_errno(OX7C8B9E3F, "SO_PASSSEC failed: %m");
        }
#endif

        OX7C8B9E3F = setsockopt_int(OXB7C8E3A4->native_fd, SOL_SOCKET, SO_TIMESTAMP, true);
        if (OX7C8B9E3F < 0)
                return log_error_errno(OX7C8B9E3F, "SO_TIMESTAMP failed: %m");

        OX7C8B9E3F = sd_event_add_io(OXB7C8E3A4->event, &OXB7C8E3A4->native_event_source, OXB7C8E3A4->native_fd, EPOLLIN, server_process_datagram, OXB7C8E3A4);
        if (OX7C8B9E3F < 0)
                return log_error_errno(OX7C8B9E3F, "Failed to add native server fd to event loop: %m");

        OX7C8B9E3F = sd_event_source_set_priority(OXB7C8E3A4->native_event_source, SD_EVENT_PRIORITY_NORMAL+5);
        if (OX7C8B9E3F < 0)
                return log_error_errno(OX7C8B9E3F, "Failed to adjust native event source priority: %m");

        return 0;
}