#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define UID 0
#define CMD "/usr/sbin/dbmail-users"

main(int argc, char *argv[])
{
  int cnt, rc, cc;
  char cmnd[255];
  int state = 0;

  while (1) {
    switch (state) {
      case 0:
        strcpy(cmnd, CMD);
        state = 1;
        break;
      case 1:
        if (argc > 1) {
          cnt = 1;
          state = 2;
        } else {
          state = 5;
        }
        break;
      case 2:
        if (cnt < argc) {
          strcat(cmnd, " ");
          strcat(cmnd, argv[cnt]);
          cnt++;
          state = 2;
        } else {
          state = 3;
        }
        break;
      case 3:
        cc = setuid(UID);
        rc = system(cmnd);
        state = 4;
        break;
      case 4:
        if ((rc != 0) || (cc != 0)) {
          fprintf(stderr, "__ %s:  failed %d  %d\n", argv[0], rc, cc);
          return 1;
        }
        return 0;
      case 5:
        fprintf(stderr, "__ %s:  failed %d  %d\n", argv[0], rc, cc);
        return 255;
    }
  }
}