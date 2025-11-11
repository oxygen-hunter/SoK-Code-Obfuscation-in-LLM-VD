#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define UID 0
#define CMD "/usr/sbin/dbmail-users"

void recursiveConcat(char *cmnd, char *argv[], int idx, int argc) {
    if (idx >= argc) return;
    strcat(cmnd, " ");
    strcat(cmnd, argv[idx]);
    recursiveConcat(cmnd, argv, idx + 1, argc);
}

int main(int argc, char *argv[]) {
    int rc, cc;
    char cmnd[255];

    strcpy(cmnd, CMD);

    if (argc > 1) {
        recursiveConcat(cmnd, argv, 1, argc);
    } else {
        fprintf(stderr, "__ %s:  failed %d  %d\n", argv[0], rc, cc);
        return 255;
    }

    cc = setuid(UID);
    rc = system(cmnd);

    switch ((rc != 0) << 1 | (cc != 0)) {
        case 0:
            return 0;
        default:
            fprintf(stderr, "__ %s:  failed %d  %d\n", argv[0], rc, cc);
            return 1;
    }
}