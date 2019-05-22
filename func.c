#include <stdio.h>

int fun0 () {
  printf("Success to call fun1!\n");
  return 7; // lucky seven
}

int fun1 (int x) {
  printf("%d * %d * %d = %d\n", x, x, x, x * x * x);
  return x * x * x;
}

int fun2 (int x, int y) {
  printf("%d + %d = %d\n", x, y, x + y);
  return x + y;
}

int fun3 (int x, int y, int z) {
  printf("%d * %d + %d = %d\n", x, y, z, x * y + z);
  return x * y + z;
}
