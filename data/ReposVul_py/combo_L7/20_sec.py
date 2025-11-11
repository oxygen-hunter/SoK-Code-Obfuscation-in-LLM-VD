#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_PATH_LEN 1024

typedef struct {
    int warning_counter;
    char *logpath;
    int quiet;
    char **path;
    char **forbidden;
    char **allowed;
    char **overssh;
    char **sudo_commands;
    char *home_path;
    char *promptprint;
} Config;

int warn_count(const char *messagetype, const char *command, Config *conf, int strict, int ssh) {
    FILE *log = fopen(conf->logpath, "a");
    if (!ssh) {
        if (strict) {
            conf->warning_counter--;
            if (conf->warning_counter < 0) {
                fprintf(log, "*** forbidden %s -> \"%s\"\n", messagetype, command);
                fprintf(log, "*** Kicked out\n");
                fclose(log);
                exit(1);
            } else {
                fprintf(log, "*** forbidden %s -> \"%s\"\n", messagetype, command);
                fprintf(stderr, "*** You have %d warning(s) left, before getting kicked out.\n", conf->warning_counter);
                fprintf(log, "*** User warned, counter: %d\n", conf->warning_counter);
                fprintf(stderr, "This incident has been reported.\n");
            }
        } else {
            if (!conf->quiet) {
                fprintf(log, "*** forbidden %s: %s\n", messagetype, command);
            }
        }
    }
    fclose(log);
    return 1;
}

int check_path(char *line, Config *conf, int completion, int ssh, int strict) {
    char allowed_path_re[MAX_PATH_LEN];
    char denied_path_re[MAX_PATH_LEN];
    strcpy(allowed_path_re, conf->path[0]);
    strcpy(denied_path_re, conf->path[1]);

    char *token;
    regex_t sep;
    regcomp(&sep, "\\ |;|\\||&", REG_EXTENDED);
    
    char *saveptr;
    token = strtok_r(line, " \t", &saveptr);

    while (token != NULL) {
        char item[MAX_PATH_LEN];
        strcpy(item, token);

        regmatch_t pmatch[1];
        regex_t quotes;
        regcomp(&quotes, "^['\"`]|['\"`]$", REG_EXTENDED);
        if (!regexec(&quotes, item, 1, pmatch, 0)) {
            memmove(item, item + 1, strlen(item));
            item[strlen(item) - 1] = '\0';
        }
        regfree(&quotes);

        regex_t dollar_braces;
        regcomp(&dollar_braces, "^\\$[\\(\\{]|[\\)\\}]$", REG_EXTENDED);
        if (!regexec(&dollar_braces, item, 1, pmatch, 0)) {
            memmove(item, item + 1, strlen(item));
            item[strlen(item) - 1] = '\0';
        }
        regfree(&dollar_braces);

        char *home = getenv("HOME");
        if (item[0] == '~') {
            char expanded[MAX_PATH_LEN];
            snprintf(expanded, sizeof(expanded), "%s%s", home, item + 1);
            strcpy(item, expanded);
        }

        regex_t shell_chars;
        regcomp(&shell_chars, "\\$|\\*|\\?", REG_EXTENDED);
        if (!regexec(&shell_chars, item, 0, NULL, 0)) {
            char command[MAX_CMD_LEN];
            snprintf(command, sizeof(command), "`which echo` %s", item);
            FILE *p = popen(command, "r");
            if (p == NULL) {
                fprintf(stderr, "Error executing command: %s\n", strerror(errno));
                return 1;
            }
            if (fgets(item, sizeof(item), p) == NULL) {
                fclose(p);
                fprintf(stderr, "*** Internal error: command not executed\n");
                return 1;
            }
            item[strcspn(item, "\n")] = '\0';
            fclose(p);
        }
        regfree(&shell_chars);

        char tomatch[MAX_PATH_LEN];
        realpath(item, tomatch);
        if (access(tomatch, F_OK) != 0) {
            strcat(tomatch, "/");
        }

        regex_t allowed_re;
        regcomp(&allowed_re, allowed_path_re, REG_EXTENDED);
        int match_allowed = !regexec(&allowed_re, tomatch, 0, NULL, 0);
        regfree(&allowed_re);

        regex_t denied_re;
        regcomp(&denied_re, denied_path_re, REG_EXTENDED);
        int match_denied = denied_path_re[0] && !regexec(&denied_re, tomatch, 0, NULL, 0);
        regfree(&denied_re);

        if (!match_allowed || match_denied) {
            if (!completion) {
                return warn_count("path", tomatch, conf, strict, ssh);
            }
            return 1;
        }

        token = strtok_r(NULL, " \t", &saveptr);
    }

    return 0;
}

int check_secure(char *line, Config *conf, int strict, int ssh) {
    char oline[MAX_CMD_LEN];
    strcpy(oline, line);

    line = strtok(line, " \t");

    int returncode = 0;

    regex_t control_chars;
    regcomp(&control_chars, "[\\x01-\\x1F\\x7F]", REG_EXTENDED);
    if (!regexec(&control_chars, oline, 0, NULL, 0)) {
        return warn_count("control char", oline, conf, strict, ssh);
    }
    regfree(&control_chars);

    regex_t forbidden_re;
    regcomp(&forbidden_re, ".*(&|\\|)", REG_EXTENDED);
    while (line != NULL) {
        if (!regexec(&forbidden_re, line, 0, NULL, 0)) {
            return warn_count("syntax", oline, conf, strict, ssh);
        }
        line = strtok(NULL, " \t");
    }
    regfree(&forbidden_re);

    return 0;
}

int main(int argc, char *argv[]) {
    Config conf;
    conf.warning_counter = 3;
    conf.logpath = "/tmp/lshell.log";
    conf.quiet = 0;
    conf.path = (char *[]){"^/usr/bin", "^/bin"};
    conf.forbidden = (char *[]){";", "&", "|"};
    conf.allowed = (char *[]){"ls", "echo", "cat"};
    conf.overssh = (char *[]){"ssh", "scp"};
    conf.sudo_commands = (char *[]){"apt-get", "service"};
    conf.home_path = "/home/user";
    conf.promptprint = "lshell";

    check_secure("ls /usr/bin", &conf, 1, 0);
    return 0;
}