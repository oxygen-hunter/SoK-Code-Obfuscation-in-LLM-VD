/* SPDX-License-Identifier: MIT */
#include "slirp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static inline int get_session_usage_state(struct tftp_session *spt) { return (spt->slirp != NULL); }
static inline void refresh_session(struct tftp_session *spt) { spt->timestamp = curtime; }

static void terminate_session(struct tftp_session *spt) {
    if (spt->fd >= 0) { close(spt->fd); spt->fd = -1; }
    g_free(spt->filename);
    spt->slirp = NULL;
}

static int allocate_session(Slirp *slirp, struct sockaddr_storage *srcsas, struct tftp_t *tp) {
    struct tftp_session *spt;
    int k;
    for (k = 0; k < TFTP_SESSIONS_MAX; k++) {
        spt = &slirp->tftp_sessions[k];
        if (!get_session_usage_state(spt)) goto found;
        if ((int)(curtime - spt->timestamp) > 5000) {
            terminate_session(spt);
            goto found;
        }
    }
    return -1;
found:
    memset(spt, 0, sizeof(*spt));
    memcpy(&spt->client_addr, srcsas, sockaddr_size(srcsas));
    spt->fd = -1;
    spt->block_size = 512;
    spt->client_port = tp->udp.uh_sport;
    spt->slirp = slirp;
    refresh_session(spt);
    return k;
}

static int find_session(Slirp *slirp, struct sockaddr_storage *srcsas, struct tftp_t *tp) {
    struct tftp_session *spt;
    int k;
    for (k = 0; k < TFTP_SESSIONS_MAX; k++) {
        spt = &slirp->tftp_sessions[k];
        if (get_session_usage_state(spt)) {
            if (sockaddr_equal(&spt->client_addr, srcsas)) {
                if (spt->client_port == tp->udp.uh_sport) {
                    return k;
                }
            }
        }
    }
    return -1;
}

static int read_data(struct tftp_session *spt, uint32_t block_nr, uint8_t *buf, int len) {
    int bytes_read = 0;
    if (spt->fd < 0) { spt->fd = open(spt->filename, O_RDONLY | O_BINARY); }
    if (spt->fd < 0) { return -1; }
    if (len) {
        lseek(spt->fd, block_nr * spt->block_size, SEEK_SET);
        bytes_read = read(spt->fd, buf, len);
    }
    return bytes_read;
}

static struct tftp_t *prepare_mbuf_data(struct tftp_session *spt, struct mbuf *m) {
    struct tftp_t *tp;
    memset(m->m_data, 0, m->m_size);
    m->m_data += IF_MAXLINKHDR;
    if (spt->client_addr.ss_family == AF_INET6) {
        m->m_data += sizeof(struct ip6);
    } else {
        m->m_data += sizeof(struct ip);
    }
    tp = (void *)m->m_data;
    m->m_data += sizeof(struct udphdr);
    return tp;
}

static void udp_output(struct tftp_session *spt, struct mbuf *m, struct tftp_t *recv_tp) {
    if (spt->client_addr.ss_family == AF_INET6) {
        struct sockaddr_in6 sa6, da6;
        sa6.sin6_addr = spt->slirp->vhost_addr6;
        sa6.sin6_port = recv_tp->udp.uh_dport;
        da6.sin6_addr = ((struct sockaddr_in6 *)&spt->client_addr)->sin6_addr;
        da6.sin6_port = spt->client_port;
        udp6_output(NULL, m, &sa6, &da6);
    } else {
        struct sockaddr_in sa4, da4;
        sa4.sin_addr = spt->slirp->vhost_addr;
        sa4.sin_port = recv_tp->udp.uh_dport;
        da4.sin_addr = ((struct sockaddr_in *)&spt->client_addr)->sin_addr;
        da4.sin_port = spt->client_port;
        udp_output(NULL, m, &sa4, &da4, IPTOS_LOWDELAY);
    }
}

static int send_oack(struct tftp_session *spt, const char *keys[], uint32_t values[], int nb, struct tftp_t *recv_tp) {
    struct mbuf *m;
    struct tftp_t *tp;
    int i, n = 0;
    m = m_get(spt->slirp);
    if (!m) return -1;
    tp = prepare_mbuf_data(spt, m);
    tp->tp_op = htons(TFTP_OACK);
    for (i = 0; i < nb; i++) {
        n += snprintf(tp->x.tp_buf + n, sizeof(tp->x.tp_buf) - n, "%s", keys[i]) + 1;
        n += snprintf(tp->x.tp_buf + n, sizeof(tp->x.tp_buf) - n, "%u", values[i]) + 1;
    }
    m->m_len = sizeof(struct tftp_t) - (TFTP_BLOCKSIZE_MAX + 2) + n - sizeof(struct udphdr);
    udp_output(spt, m, recv_tp);
    return 0;
}

static void send_error(struct tftp_session *spt, uint16_t errorcode, const char *msg, struct tftp_t *recv_tp) {
    struct mbuf *m;
    struct tftp_t *tp;
    DEBUG_TFTP("tftp error msg: %s", msg);
    m = m_get(spt->slirp);
    if (!m) { goto out; }
    tp = prepare_mbuf_data(spt, m);
    tp->tp_op = htons(TFTP_ERROR);
    tp->x.tp_error.tp_error_code = htons(errorcode);
    slirp_pstrcpy((char *)tp->x.tp_error.tp_msg, sizeof(tp->x.tp_error.tp_msg), msg);
    m->m_len = sizeof(struct tftp_t) - (TFTP_BLOCKSIZE_MAX + 2) + 3 + strlen(msg) - sizeof(struct udphdr);
    udp_output(spt, m, recv_tp);
out:
    terminate_session(spt);
}

static void send_next_block(struct tftp_session *spt, struct tftp_t *recv_tp) {
    struct mbuf *m;
    struct tftp_t *tp;
    int nobytes;
    m = m_get(spt->slirp);
    if (!m) { return; }
    tp = prepare_mbuf_data(spt, m);
    tp->tp_op = htons(TFTP_DATA);
    tp->x.tp_data.tp_block_nr = htons((spt->block_nr + 1) & 0xffff);
    nobytes = read_data(spt, spt->block_nr, tp->x.tp_data.tp_buf, spt->block_size);
    if (nobytes < 0) {
        m_free(m);
        send_error(spt, 1, "File not found", tp);
        return;
    }
    m->m_len = sizeof(struct tftp_t) - (TFTP_BLOCKSIZE_MAX - nobytes) - sizeof(struct udphdr);
    udp_output(spt, m, recv_tp);
    if (nobytes == spt->block_size) {
        refresh_session(spt);
    } else {
        terminate_session(spt);
    }
    spt->block_nr++;
}

static void handle_rrq(Slirp *slirp, struct sockaddr_storage *srcsas, struct tftp_t *tp, int pktlen) {
    struct tftp_session *spt;
    int s, k;
    size_t prefix_len;
    char *req_fname;
    const char *option_name[2];
    uint32_t option_value[2];
    int nb_options = 0;
    s = find_session(slirp, srcsas, tp);
    if (s >= 0) { terminate_session(&slirp->tftp_sessions[s]); }
    s = allocate_session(slirp, srcsas, tp);
    if (s < 0) { return; }
    spt = &slirp->tftp_sessions[s];
    if (!slirp->tftp_prefix) {
        send_error(spt, 2, "Access violation", tp);
        return;
    }
    k = 0;
    pktlen -= offsetof(struct tftp_t, x.tp_buf);
    prefix_len = strlen(slirp->tftp_prefix);
    spt->filename = g_malloc(prefix_len + TFTP_FILENAME_MAX + 2);
    memcpy(spt->filename, slirp->tftp_prefix, prefix_len);
    spt->filename[prefix_len] = '/';
    req_fname = spt->filename + prefix_len + 1;
    while (1) {
        if (k >= TFTP_FILENAME_MAX || k >= pktlen) {
            send_error(spt, 2, "Access violation", tp);
            return;
        }
        req_fname[k] = tp->x.tp_buf[k];
        if (req_fname[k++] == '\0') { break; }
    }
    DEBUG_TFTP("tftp rrq file: %s", req_fname);
    if ((pktlen - k) < 6) {
        send_error(spt, 2, "Access violation", tp);
        return;
    }
    if (strcasecmp(&tp->x.tp_buf[k], "octet") != 0) {
        send_error(spt, 4, "Unsupported transfer mode", tp);
        return;
    }
    k += 6;
    if (!strncmp(req_fname, "../", 3) || req_fname[strlen(req_fname) - 1] == '/' || strstr(req_fname, "/../")) {
        send_error(spt, 2, "Access violation", tp);
        return;
    }
    if (read_data(spt, 0, NULL, 0) < 0) {
        send_error(spt, 1, "File not found", tp);
        return;
    }
    if (tp->x.tp_buf[pktlen - 1] != 0) {
        send_error(spt, 2, "Access violation", tp);
        return;
    }
    while (k < pktlen && nb_options < G_N_ELEMENTS(option_name)) {
        const char *key, *value;
        key = &tp->x.tp_buf[k];
        k += strlen(key) + 1;
        if (k >= pktlen) {
            send_error(spt, 2, "Access violation", tp);
            return;
        }
        value = &tp->x.tp_buf[k];
        k += strlen(value) + 1;
        if (strcasecmp(key, "tsize") == 0) {
            int tsize = atoi(value);
            struct stat stat_p;
            if (tsize == 0) {
                if (stat(spt->filename, &stat_p) == 0) tsize = stat_p.st_size;
                else {
                    send_error(spt, 1, "File not found", tp);
                    return;
                }
            }
            option_name[nb_options] = "tsize";
            option_value[nb_options] = tsize;
            nb_options++;
        } else if (strcasecmp(key, "blksize") == 0) {
            int blksize = atoi(value);
            if (blksize > 0) {
                spt->block_size = MIN(blksize, TFTP_BLOCKSIZE_MAX);
                option_name[nb_options] = "blksize";
                option_value[nb_options] = spt->block_size;
                nb_options++;
            }
        }
    }
    if (nb_options > 0) {
        assert(nb_options <= G_N_ELEMENTS(option_name));
        send_oack(spt, option_name, option_value, nb_options, tp);
        return;
    }
    spt->block_nr = 0;
    send_next_block(spt, tp);
}

static void handle_ack(Slirp *slirp, struct sockaddr_storage *srcsas, struct tftp_t *tp, int pktlen) {
    int s;
    s = find_session(slirp, srcsas, tp);
    if (s < 0) { return; }
    send_next_block(&slirp->tftp_sessions[s], tp);
}

static void handle_error(Slirp *slirp, struct sockaddr_storage *srcsas, struct tftp_t *tp, int pktlen) {
    int s;
    s = find_session(slirp, srcsas, tp);
    if (s < 0) { return; }
    terminate_session(&slirp->tftp_sessions[s]);
}

void tftp_input(struct sockaddr_storage *srcsas, struct mbuf *m) {
    struct tftp_t *tp = (struct tftp_t *)m->m_data;
    switch (ntohs(tp->tp_op)) {
    case TFTP_RRQ:
        handle_rrq(m->slirp, srcsas, tp, m->m_len);
        break;
    case TFTP_ACK:
        handle_ack(m->slirp, srcsas, tp, m->m_len);
        break;
    case TFTP_ERROR:
        handle_error(m->slirp, srcsas, tp, m->m_len);
        break;
    }
}