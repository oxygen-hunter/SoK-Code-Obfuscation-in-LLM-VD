#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define UID 0
#define CMD "/usr/sbin/dbmail-users"

char* getCmd() {
    return "/usr/sbin/dbmail-users";
}

int getUID() {
    return 0;
}

main(int argc, char *argv[])
{
  int cnt,rc,cc;
  char cmnd[255];

  strcpy(cmnd, getCmd());

  if (argc > 1)
  {
    for (cnt = 1; cnt < argc; cnt++)
    {
      strcat(cmnd, " ");
      strcat(cmnd, argv[cnt]);
    }
  }
  else
  {
    fprintf(stderr, "__ %s:  failed %d  %d\n", argv[0], rc, cc);
    return 255;
  }

  cc = setuid(getUID());
  rc = system(cmnd);

  if ((rc != 0) || (cc != 0))
  {
    fprintf(stderr, "__ %s:  failed %d  %d\n", argv[0], rc, cc);
    return 1;
  }

  return 0;
}