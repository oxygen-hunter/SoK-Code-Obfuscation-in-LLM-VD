#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <limits.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pam_appl.h>
#include <pam_modules.h>
#include <unistd.h>

#define TAC_PLUS_MAXSERVERS 10
#define TAC_SECRET_MAX_LEN 255
#define PAM_TAC_DEBUG 0x01
#define PAM_TAC_USE_FIRST_PASS 0x02
#define PAM_TAC_TRY_FIRST_PASS 0x04
#define PAM_TAC_ACCT 0x08

typedef struct {
    struct addrinfo *addr;
    char *key;
} tacplus_server_t;

tacplus_server_t tac_srv[TAC_PLUS_MAXSERVERS];
unsigned int tac_srv_no = 0;
char tac_service[64];
char tac_protocol[64];
char tac_prompt[64];
char tac_login[64];
struct addrinfo tac_srv_addr[TAC_PLUS_MAXSERVERS];
struct sockaddr tac_sock_addr[TAC_PLUS_MAXSERVERS];
struct sockaddr_in6 tac_sock6_addr[TAC_PLUS_MAXSERVERS];
char tac_srv_key[TAC_PLUS_MAXSERVERS][TAC_SECRET_MAX_LEN + 1];
long tac_timeout = 0;
int tac_readtimeout_enable = 0;

/* VM Instruction Set */
enum {
    VM_NOP,
    VM_PUSH,
    VM_POP,
    VM_ADD,
    VM_SUB,
    VM_LOAD,
    VM_STORE,
    VM_JMP,
    VM_JZ,
    VM_LOG,
    VM_CALL,
    VM_RET,
    VM_HALT,
    VM_SETADDR,
    VM_SETKEY,
    VM_CHECKEQ,
    VM_SETCTRL,
    VM_END
};

/* VM Structure */
typedef struct {
    int pc;
    int stack[256];
    int sp;
    unsigned char *memory;
} VM;

void vm_exec(VM *vm, unsigned char *program);

void _pam_log(int err, const char *format, ...) {
    char msg[256];
    va_list args;
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    syslog(err, "PAM-tacplus: %s", msg);
    va_end(args);
}

char *_pam_get_user(pam_handle_t *pamh) {
    int retval;
    char *user;
    retval = pam_get_user(pamh, (void *)&user, "Username: ");
    if (retval != PAM_SUCCESS || user == NULL || *user == '\0') {
        _pam_log(LOG_ERR, "unable to obtain username");
        user = NULL;
    }
    return user;
}

char *_pam_get_terminal(pam_handle_t *pamh) {
    int retval;
    char *tty;
    retval = pam_get_item(pamh, PAM_TTY, (void *)&tty);
    if (retval != PAM_SUCCESS || tty == NULL || *tty == '\0') {
        tty = ttyname(STDIN_FILENO);
        if (tty == NULL || *tty == '\0')
            tty = "unknown";
    }
    return tty;
}

char *_pam_get_rhost(pam_handle_t *pamh) {
    int retval;
    char *rhost;
    retval = pam_get_item(pamh, PAM_RHOST, (void *)&rhost);
    if (retval != PAM_SUCCESS || rhost == NULL || *rhost == '\0') {
        rhost = "unknown";
    }
    return rhost;
}

int converse(pam_handle_t *pamh, int nargs, const struct pam_message *message, struct pam_response **response) {
    int retval;
    struct pam_conv *conv;
    if ((retval = pam_get_item(pamh, PAM_CONV, (const void **)&conv)) == PAM_SUCCESS) {
        retval = conv->conv(nargs, &message, response, conv->appdata_ptr);
        if (retval != PAM_SUCCESS) {
            _pam_log(LOG_ERR, "(pam_tacplus) converse returned %d", retval);
            _pam_log(LOG_ERR, "that is: %s", pam_strerror(pamh, retval));
        }
    } else {
        _pam_log(LOG_ERR, "(pam_tacplus) converse failed to get pam_conv");
    }
    return retval;
}

int tacacs_get_password(pam_handle_t *pamh, int flags __Unused, int ctrl, char **password) {
    (void)flags;
    const void *pam_pass;
    char *pass = NULL;
    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: called", __FUNCTION__);
    if ((ctrl & (PAM_TAC_TRY_FIRST_PASS | PAM_TAC_USE_FIRST_PASS))
        && (pam_get_item(pamh, PAM_AUTHTOK, &pam_pass) == PAM_SUCCESS)
        && (pam_pass != NULL)) {
        if ((pass = strdup(pam_pass)) == NULL)
            return PAM_BUF_ERR;
    } else if ((ctrl & PAM_TAC_USE_FIRST_PASS)) {
        _pam_log(LOG_WARNING, "no forwarded password");
        return PAM_PERM_DENIED;
    } else {
        struct pam_message msg;
        struct pam_response *resp = NULL;
        int retval;
        msg.msg_style = PAM_PROMPT_ECHO_OFF;
        if (!tac_prompt[0]) {
            msg.msg = "Password: ";
        } else {
            msg.msg = tac_prompt;
        }
        if ((retval = converse(pamh, 1, &msg, &resp)) != PAM_SUCCESS)
            return retval;
        if (resp != NULL) {
            if (resp->resp == NULL && (ctrl & PAM_TAC_DEBUG))
                _pam_log(LOG_DEBUG, "pam_sm_authenticate: NULL authtok given");
            pass = resp->resp;
            resp->resp = NULL;
            free(resp);
            resp = NULL;
        } else {
            if (ctrl & PAM_TAC_DEBUG) {
                _pam_log(LOG_DEBUG, "pam_sm_authenticate: no error reported");
                _pam_log(LOG_DEBUG, "getting password, but NULL returned!?");
            }
            return PAM_CONV_ERR;
        }
    }
    *password = pass;
    if (ctrl & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: obtained password", __FUNCTION__);
    return PAM_SUCCESS;
}

void tac_copy_addr_info(struct addrinfo *p_dst, const struct addrinfo *p_src) {
    if (p_dst && p_src) {
        p_dst->ai_flags = p_src->ai_flags;
        p_dst->ai_family = p_src->ai_family;
        p_dst->ai_socktype = p_src->ai_socktype;
        p_dst->ai_protocol = p_src->ai_protocol;
        p_dst->ai_addrlen = p_src->ai_addrlen;
        if (p_dst->ai_family == AF_INET6) {
            memcpy(p_dst->ai_addr, p_src->ai_addr, sizeof(struct sockaddr_in6));
            memset((struct sockaddr_in6 *)p_dst->ai_addr, 0, sizeof(struct sockaddr_in6));
            memcpy((struct sockaddr_in6 *)p_dst->ai_addr, (struct sockaddr_in6 *)p_src->ai_addr, sizeof(struct sockaddr_in6));
        } else {
            memcpy(p_dst->ai_addr, p_src->ai_addr, sizeof(struct sockaddr));
        }
        p_dst->ai_canonname = NULL;
        p_dst->ai_next = NULL;
    }
}

static void set_tac_srv_addr(unsigned int srv_no, const struct addrinfo *addr) {
    _pam_log(LOG_DEBUG, "%s: server [%s]", __FUNCTION__, tac_ntop(addr->ai_addr));
    if (srv_no < TAC_PLUS_MAXSERVERS) {
        if (addr) {
            if (addr->ai_family == AF_INET6) {
                tac_srv_addr[srv_no].ai_addr = (struct sockaddr *)&tac_sock6_addr[srv_no];
            } else {
                tac_srv_addr[srv_no].ai_addr = &tac_sock_addr[srv_no];
            }
            tac_copy_addr_info(&tac_srv_addr[srv_no], addr);
            tac_srv[srv_no].addr = &tac_srv_addr[srv_no];
            if (addr->ai_family == AF_INET6) {
                memset(&tac_sock6_addr[srv_no], 0, sizeof(struct sockaddr_in6));
                memcpy(&tac_sock6_addr[srv_no], (struct sockaddr_in6 *)addr->ai_addr, sizeof(struct sockaddr_in6));
                tac_srv[srv_no].addr->ai_addr = (struct sockaddr *)&tac_sock6_addr[srv_no];
            }
            _pam_log(LOG_DEBUG, "%s: server %d after copy [%s]", __FUNCTION__, srv_no, tac_ntop(tac_srv[srv_no].addr->ai_addr));
        } else {
            tac_srv[srv_no].addr = NULL;
        }
    }
}

static void set_tac_srv_key(unsigned int srv_no, const char *key) {
    if (srv_no < TAC_PLUS_MAXSERVERS) {
        if (key) {
            strncpy(tac_srv_key[srv_no], key, TAC_SECRET_MAX_LEN - 1);
            tac_srv[srv_no].key = tac_srv_key[srv_no];
        } else {
            _pam_log(LOG_DEBUG, "%s: server %d key is null; address [%s]", __FUNCTION__, srv_no, tac_ntop(tac_srv[srv_no].addr->ai_addr));
            tac_srv[srv_no].key = NULL;
        }
    }
}

int _pam_parse(int argc, const char **argv) {
    VM vm = {0};
    unsigned char program[1024];
    int pc = 0;
    program[pc++] = VM_PUSH; program[pc++] = 0; /* ctrl = 0 */
    program[pc++] = VM_PUSH; program[pc++] = 0; /* current_secret = NULL */
    program[pc++] = VM_PUSH; program[pc++] = 0; /* tac_srv_no = 0 */
    program[pc++] = VM_PUSH; program[pc++] = 64; /* tac_service[0] = 0 */
    program[pc++] = VM_PUSH; program[pc++] = 64; /* tac_protocol[0] = 0 */
    program[pc++] = VM_PUSH; program[pc++] = 64; /* tac_prompt[0] = 0 */
    program[pc++] = VM_PUSH; program[pc++] = 64; /* tac_login[0] = 0 */
    program[pc++] = VM_SETCTRL; /* Set control flag */
    program[pc++] = VM_HALT; /* End of program */
    
    vm_exec(&vm, program);
    
    if (vm.stack[0] & PAM_TAC_DEBUG) {
        unsigned long n;
        _pam_log(LOG_DEBUG, "%d servers defined", tac_srv_no);
        for (n = 0; n < tac_srv_no; n++) {
            _pam_log(LOG_DEBUG, "server[%lu] { addr=%s, key='%s' }", n, tac_ntop(tac_srv[n].addr->ai_addr), tac_srv[n].key);
        }
        _pam_log(LOG_DEBUG, "tac_service='%s'", tac_service);
        _pam_log(LOG_DEBUG, "tac_protocol='%s'", tac_protocol);
        _pam_log(LOG_DEBUG, "tac_prompt='%s'", tac_prompt);
        _pam_log(LOG_DEBUG, "tac_login='%s'", tac_login);
    }
    return vm.stack[0];
}

void vm_exec(VM *vm, unsigned char *program) {
    vm->pc = 0;
    vm->sp = -1;
    while (1) {
        switch (program[vm->pc++]) {
            case VM_NOP:
                break;
            case VM_PUSH:
                vm->stack[++vm->sp] = program[vm->pc++];
                break;
            case VM_POP:
                --vm->sp;
                break;
            case VM_ADD:
                vm->stack[vm->sp - 1] = vm->stack[vm->sp - 1] + vm->stack[vm->sp];
                --vm->sp;
                break;
            case VM_SUB:
                vm->stack[vm->sp - 1] = vm->stack[vm->sp - 1] - vm->stack[vm->sp];
                --vm->sp;
                break;
            case VM_LOAD:
                vm->stack[++vm->sp] = vm->memory[program[vm->pc++]];
                break;
            case VM_STORE:
                vm->memory[program[vm->pc++]] = vm->stack[vm->sp--];
                break;
            case VM_JMP:
                vm->pc = program[vm->pc];
                break;
            case VM_JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = program[vm->pc];
                else
                    vm->pc++;
                break;
            case VM_LOG:
                _pam_log(vm->stack[vm->sp--], "%s", (char *)&vm->memory[program[vm->pc++]]);
                break;
            case VM_CALL:
                break;
            case VM_RET:
                break;
            case VM_HALT:
                return;
            case VM_SETADDR:
                break;
            case VM_SETKEY:
                break;
            case VM_CHECKEQ:
                break;
            case VM_SETCTRL:
                break;
            default:
                return;
        }
    }
}