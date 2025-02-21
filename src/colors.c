#include <stdio.h>
#include "colors.h"

void textcolor(int attr, int fg) {
  if (attr == CLR_CLEAR) {
    printf("\x1B[0m");
    return;
  }
  
  printf("%c[%d;%dm", 0x1B, attr, fg + 30);
}

