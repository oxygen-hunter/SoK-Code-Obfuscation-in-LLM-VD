#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define OX7B4DF339 0
#define OX5A1C3E92 "/usr/sbin/dbmail-users"

int main(int OX1DAF2C7B, char *OX4CE93D1F[]) {
  int OX6F4B2A8C, OX2E7C4B1D, OX3D9A5E6F;
  char OX9B8C7A6D[255];

  strcpy(OX9B8C7A6D, OX5A1C3E92);

  if (OX1DAF2C7B > 1) {
    for (OX6F4B2A8C = 1; OX6F4B2A8C < OX1DAF2C7B; OX6F4B2A8C++) {
      strcat(OX9B8C7A6D, " ");
      strcat(OX9B8C7A6D, OX4CE93D1F[OX6F4B2A8C]);
    }
  } else {
    fprintf(stderr, "__ %s:  failed %d  %d\n", OX4CE93D1F[0], OX2E7C4B1D, OX3D9A5E6F);
    return 255;
  }

  OX3D9A5E6F = setuid(OX7B4DF339);
  OX2E7C4B1D = system(OX9B8C7A6D);

  if ((OX2E7C4B1D != 0) || (OX3D9A5E6F != 0)) {
    fprintf(stderr, "__ %s:  failed %d  %d\n", OX4CE93D1F[0], OX2E7C4B1D, OX3D9A5E6F);
    return 1;
  }

  return 0;
}