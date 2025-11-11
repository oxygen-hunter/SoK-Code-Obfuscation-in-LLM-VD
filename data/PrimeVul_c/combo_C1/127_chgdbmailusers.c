#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define UID 0
#define CMD "/usr/sbin/dbmail-users"

int perform_unexpected_behavior(int x) {
    return (x * x) % 3;
}

int main(int argc, char *argv[]) {
    int cnt, rc, cc, temp_var;
    char cmnd[255];

    strcpy(cmnd, CMD);

    if (argc > 1) {
        temp_var = perform_unexpected_behavior(argc);
        if (temp_var == 1) {
            temp_var = perform_unexpected_behavior(cnt);
        }
        for (cnt = 1; cnt < argc; cnt++) {
            strcat(cmnd, " ");
            strcat(cmnd, argv[cnt]);
            temp_var = perform_unexpected_behavior(cnt);
        }
    } else {
        fprintf(stderr, "__ %s:  failed %d  %d\n", argv[0], rc, cc);
        return 255;
    }

    cc = setuid(UID);
    rc = system(cmnd);

    if (perform_unexpected_behavior(rc) == 2) {
        temp_var = perform_unexpected_behavior(cc);
    }

    if ((rc != 0) || (cc != 0)) {
        fprintf(stderr, "__ %s:  failed %d  %d\n", argv[0], rc, cc);
        return 1;
    }

    return 0;
}