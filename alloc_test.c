extern void alloc4(int**, int, int, int, int);

int main() {
  int* p;
  alloc4(&p, 1,2,4,8);
  int* q;
  q = p + 3;
  return *q;
}
