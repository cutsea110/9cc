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

int fun4 (int x, int y, int z, int u) {
  printf("%d * %d + %d ^ %d = %d\n", x, y, z, u, x * y + z / u);
  return x * y + z / u;
}

int fun5 (int x, int y, int z, int u, int v) {
  printf("%d * %d + %d - %d % %d = %d\n", x, y, z, u, v, x * y + z - u % v);
  return  x * y + z - u % v;
}

int fun6 (int x, int y, int z, int u, int v, int w) {
  printf("%d * %d + %d - %d mod %d * %d = %d\n", x, y, z, u, v, w, x * y + z - (u % v) * w);
  return  x * y + z - (u % v) * w;
}
