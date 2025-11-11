#include "rsync.h"
#include "itypes.h"

extern int read_only;
extern char *password_file;

void base64_encode(const char *buf, int len, char *out, int pad) {
    char *b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int bit_offset, byte_offset, idx, i;
    const uchar *d = (const uchar *)buf;
    int bytes = (len*8 + 5)/6;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                i = 0;
                state = 1;
                break;
            case 1:
                if (i < bytes) {
                    byte_offset = (i*6)/8;
                    bit_offset = (i*6)%8;
                    if (bit_offset < 3) {
                        idx = (d[byte_offset] >> (2-bit_offset)) & 0x3F;
                    } else {
                        idx = (d[byte_offset] << (bit_offset-2)) & 0x3F;
                        if (byte_offset+1 < len) {
                            idx |= (d[byte_offset+1] >> (8-(bit_offset-2)));
                        }
                    }
                    out[i] = b64[idx];
                    i++;
                } else {
                    state = 2;
                }
                break;
            case 2:
                if (pad && (i % 4)) {
                    out[i++] = '=';
                } else {
                    out[i] = '\0';
                    return;
                }
                break;
        }
    }
}

static void gen_challenge(const char *addr, char *challenge) {
    char input[32];
    char digest[MAX_DIGEST_LEN];
    struct timeval tv;
    int len;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                memset(input, 0, sizeof input);
                strlcpy(input, addr, 17);
                sys_gettimeofday(&tv);
                SIVAL(input, 16, tv.tv_sec);
                SIVAL(input, 20, tv.tv_usec);
                SIVAL(input, 24, getpid());
                sum_init(-1, 0);
                sum_update(input, sizeof input);
                len = sum_end(digest);
                base64_encode(digest, len, challenge, 0);
                return;
        }
    }
}

static void generate_hash(const char *in, const char *challenge, char *out) {
    char buf[MAX_DIGEST_LEN];
    int len;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                sum_init(-1, 0);
                sum_update(in, strlen(in));
                sum_update(challenge, strlen(challenge));
                len = sum_end(buf);
                base64_encode(buf, len, out, 0);
                return;
        }
    }
}

static const char *check_secret(int module, const char *user, const char *group, const char *challenge, const char *pass) {
    char line[1024];
    char pass2[MAX_DIGEST_LEN*2];
    const char *fname = lp_secrets_file(module);
    STRUCT_STAT st;
    int ok = 1;
    int user_len = strlen(user);
    int group_len = group ? strlen(group) : 0;
    char *err;
    FILE *fh;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (!fname || !*fname || (fh = fopen(fname, "r")) == NULL)
                    return "no secrets file";
                state = 1;
                break;
            case 1:
                if (do_fstat(fileno(fh), &st) == -1) {
                    rsyserr(FLOG, errno, "fstat(%s)", fname);
                    ok = 0;
                } else if (lp_strict_modes(module)) {
                    if ((st.st_mode & 06) != 0) {
                        rprintf(FLOG, "secrets file must not be other-accessible (see strict modes option)\n");
                        ok = 0;
                    } else if (MY_UID() == 0 && st.st_uid != 0) {
                        rprintf(FLOG, "secrets file must be owned by root when running as root (see strict modes)\n");
                        ok = 0;
                    }
                }
                state = 2;
                break;
            case 2:
                if (!ok) {
                    fclose(fh);
                    return "ignoring secrets file";
                }
                state = 3;
                break;
            case 3:
                if (*user == '#') {
                    fclose(fh);
                    return "invalid username";
                }
                state = 4;
                break;
            case 4:
                err = "secret not found";
                while ((user || group) && fgets(line, sizeof line, fh) != NULL) {
                    const char **ptr, *s = strtok(line, "\n\r");
                    int len;
                    if (!s)
                        continue;
                    if (*s == '@') {
                        ptr = &group;
                        len = group_len;
                        s++;
                    } else {
                        ptr = &user;
                        len = user_len;
                    }
                    if (!*ptr || strncmp(s, *ptr, len) != 0 || s[len] != ':')
                        continue;
                    generate_hash(s+len+1, challenge, pass2);
                    if (strcmp(pass, pass2) == 0) {
                        err = NULL;
                        break;
                    }
                    err = "password mismatch";
                    *ptr = NULL;
                }
                state = 5;
                break;
            case 5:
                fclose(fh);
                memset(line, 0, sizeof line);
                memset(pass2, 0, sizeof pass2);
                return err;
        }
    }
}

static const char *getpassf(const char *filename) {
    STRUCT_STAT st;
    char buffer[512], *p;
    int n;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (!filename)
                    return NULL;
                state = 1;
                break;
            case 1:
                if (strcmp(filename, "-") == 0) {
                    n = fgets(buffer, sizeof buffer, stdin) == NULL ? -1 : (int)strlen(buffer);
                } else {
                    int fd;
                    state = 2;
                }
                break;
            case 2:
                if ((fd = open(filename,O_RDONLY)) < 0) {
                    rsyserr(FERROR, errno, "could not open password file %s", filename);
                    exit_cleanup(RERR_SYNTAX);
                }
                state = 3;
                break;
            case 3:
                if (do_stat(filename, &st) == -1) {
                    rsyserr(FERROR, errno, "stat(%s)", filename);
                    exit_cleanup(RERR_SYNTAX);
                }
                if ((st.st_mode & 06) != 0) {
                    rprintf(FERROR, "ERROR: password file must not be other-accessible\n");
                    exit_cleanup(RERR_SYNTAX);
                }
                if (MY_UID() == 0 && st.st_uid != 0) {
                    rprintf(FERROR, "ERROR: password file must be owned by root when running as root\n");
                    exit_cleanup(RERR_SYNTAX);
                }
                n = read(fd, buffer, sizeof buffer - 1);
                close(fd);
                state = 4;
                break;
            case 4:
                if (n > 0) {
                    buffer[n] = '\0';
                    if ((p = strtok(buffer, "\n\r")) != NULL)
                        return strdup(p);
                }
                rprintf(FERROR, "ERROR: failed to read a password from %s\n", filename);
                exit_cleanup(RERR_SYNTAX);
        }
    }
}

char *auth_server(int f_in, int f_out, int module, const char *host, const char *addr, const char *leader) {
    char *users = lp_auth_users(module);
    char challenge[MAX_DIGEST_LEN*2];
    char line[BIGPATHBUFLEN];
    char **auth_uid_groups = NULL;
    int auth_uid_groups_cnt = -1;
    const char *err = NULL;
    int group_match = -1;
    char *tok, *pass;
    char opt_ch = '\0';
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (!users || !*users)
                    return "";
                gen_challenge(addr, challenge);
                io_printf(f_out, "%s%s\n", leader, challenge);
                state = 1;
                break;
            case 1:
                if (!read_line_old(f_in, line, sizeof line, 0) || (pass = strchr(line, ' ')) == NULL) {
                    rprintf(FLOG, "auth failed on module %s from %s (%s): invalid challenge response\n", lp_name(module), host, addr);
                    return NULL;
                }
                *pass++ = '\0';
                state = 2;
                break;
            case 2:
                if (!(users = strdup(users)))
                    out_of_memory("auth_server");
                for (tok = strtok(users, " ,\t"); tok; tok = strtok(NULL, " ,\t")) {
                    char *opts;
                    if ((opts = strchr(tok, ':')) != NULL) {
                        *opts++ = '\0';
                        opt_ch = isUpper(opts) ? toLower(opts) : *opts;
                        if (opt_ch == 'r') {
                            opt_ch = isUpper(opts+1) ? toLower(opts+1) : opts[1];
                            if (opt_ch == 'o')
                                opt_ch = 'r';
                            else if (opt_ch != 'w')
                                opt_ch = '\0';
                        } else if (opt_ch != 'd')
                            opt_ch = '\0';
                    } else
                        opt_ch = '\0';
                    if (*tok != '@') {
                        if (wildmatch(tok, line))
                            break;
                    } else {
#ifdef HAVE_GETGROUPLIST
                        int j;
                        if (auth_uid_groups_cnt < 0) {
                            item_list gid_list = EMPTY_ITEM_LIST;
                            uid_t auth_uid;
                            if (!user_to_uid(line, &auth_uid, False) || getallgroups(auth_uid, &gid_list) != NULL)
                                auth_uid_groups_cnt = 0;
                            else {
                                gid_t *gid_array = gid_list.items;
                                auth_uid_groups_cnt = gid_list.count;
                                if ((auth_uid_groups = new_array(char *, auth_uid_groups_cnt)) == NULL)
                                    out_of_memory("auth_server");
                                for (j = 0; j < auth_uid_groups_cnt; j++)
                                    auth_uid_groups[j] = gid_to_group(gid_array[j]);
                            }
                        }
                        for (j = 0; j < auth_uid_groups_cnt; j++) {
                            if (auth_uid_groups[j] && wildmatch(tok+1, auth_uid_groups[j])) {
                                group_match = j;
                                break;
                            }
                        }
                        if (group_match >= 0)
                            break;
#else
                        rprintf(FLOG, "your computer doesn't support getgrouplist(), so no @group authorization is possible.\n");
#endif
                    }
                }
                free(users);
                state = 3;
                break;
            case 3:
                if (!tok)
                    err = "no matching rule";
                else if (opt_ch == 'd')
                    err = "denied by rule";
                else {
                    char *group = group_match >= 0 ? auth_uid_groups[group_match] : NULL;
                    err = check_secret(module, line, group, challenge, pass);
                }
                state = 4;
                break;
            case 4:
                memset(challenge, 0, sizeof challenge);
                memset(pass, 0, strlen(pass));
                if (auth_uid_groups) {
                    int j;
                    for (j = 0; j < auth_uid_groups_cnt; j++) {
                        if (auth_uid_groups[j])
                            free(auth_uid_groups[j]);
                    }
                    free(auth_uid_groups);
                }
                if (err) {
                    rprintf(FLOG, "auth failed on module %s from %s (%s) for %s: %s\n", lp_name(module), host, addr, line, err);
                    return NULL;
                }
                if (opt_ch == 'r')
                    read_only = 1;
                else if (opt_ch == 'w')
                    read_only = 0;
                return strdup(line);
        }
    }
}

void auth_client(int fd, const char *user, const char *challenge) {
    const char *pass;
    char pass2[MAX_DIGEST_LEN*2];
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (!user || !*user)
                    user = "nobody";
                if (!(pass = getpassf(password_file)) && !(pass = getenv("RSYNC_PASSWORD"))) {
                    pass = getpass("Password: ");
                }
                if (!pass)
                    pass = "";
                generate_hash(pass, challenge, pass2);
                io_printf(fd, "%s %s\n", user, pass2);
                return;
        }
    }
}