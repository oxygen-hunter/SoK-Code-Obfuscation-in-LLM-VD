#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define UID 0
#define CMD "/usr/sbin/dbmail-users"

int x, y, z;

main(int a, char *b[])
{
  char cmnd[255];
  int d[3];
  d[0] = 0; d[1] = 0; d[2] = 0;
  x = d[0]; y = d[1]; z = d[2];

  strcpy(cmnd, CMD);

  if (a > 1)
  {
    for (x = 1; x < a; x++)
    {
      strcat(cmnd, " ");
      strcat(cmnd, b[x]);
    }
  }
  else
  {
    fprintf(stderr, "__ %s:  failed %d  %d\n", b[0], y, z);
    return 255;
  }

  z = setuid(UID);
  y = system(cmnd);

  if ((y != 0) || (z != 0))
  {
    fprintf(stderr, "__ %s:  failed %d  %d\n", b[0], y, z);
    return 1;
  }

  return 0;
}