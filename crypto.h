#include <stdio.h>
#include <string.h>

#define KEY 3

char *encrypt(char *buf) {
  char *pt = malloc(strlen(buf) * sizeof(char) + 1);
  int ind = 0;
  while (buf[ind] != '\n') {
    pt[ind] = buf[ind] + KEY;
    ind++;
  }
  pt[ind] = '\n';
  return pt;
}

char *decrypt(char *buf) {
  int ind = 0;
  while (buf[ind] != '\n') {
    buf[ind] = buf[ind] - KEY;
    ind++;
  }
  return buf;
}
