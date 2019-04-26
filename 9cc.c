#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
      TK_NUM = 256, // 整数トークン
      TK_EOF,       // 入力の終端を表すトークン
};

enum {
      ND_NUM = 256, // 整数のノードの型
};

typedef struct {
  int ty;           // トークンの型
  int val;          // tyがTK_NUMの場合,その数値
  char* input;      // トークン文字列(エラーメッセージ用)
} Token;

typedef struct Node {
  int ty;           // 演算子かND_NUM
  struct Node* lhs; // 左辺
  struct Node* rhs; // 右辺
  int val;          // tyがND_NUMの場合のみ使う
} Node;

typedef struct {
  void** data;      // データ本体
  int capacity;     // バッファの大きさ(data[0]~data[capacity-1]がバッファ領域)
  int len;          // ベクタに追加済みの要素数
} Vector;

Vector* new_vector();
void vec_push(Vector*, void*);
Token* add_token(Vector*, int, char*);

Node* add();
Node* mul();
Node* term();
int consume(int);
void error(char*, ...);
void DEBUG(char*, ...);
Node* new_node(int, Node*, Node*);
Node* new_node_num(int);

int expect(int, int, int);
void runtest();

int expect(int line, int expected, int actual) {
  if (expected == actual)
    return 1;
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);
}

void runtest() {
  Vector* vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++)
    vec_push(vec, (void*)i);

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);

  printf("OK\n");
}

Vector* tokens;
int pos = 0;

Vector* new_vector() {
  Vector* vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void*) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector* vec, void* elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void*) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

Token* add_token(Vector* v, int ty, char* p) {
  Token* t = malloc(sizeof(Token));
  t->ty = ty;
  t->input = p;
  vec_push(v, t);
  return t;
}

Node* new_node(int ty, Node* lhs, Node* rhs) {
  Node* node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  DEBUG("return new_node");
  return node;
}

Node* new_node_num(int val) {
  Node* node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  DEBUG("return new_node_num");
  return node;
}

int consume(int ty) {
  Token* t = tokens->data[pos];
  if (t->ty != ty)
    return 0;
  pos++;
  return 1;
}

Node* term() {
  DEBUG("Entry term");
  
  if (consume('(')) {
    DEBUG("'(' Found");
    Node* node = add();
    if (!consume(')')) {
      DEBUG("')' NOT Found");
      Token* t = tokens->data[pos];
      error("開きカッコに対応する閉じカッコがありません: %s", t->input);
    }
    DEBUG("')' Found");
    return node;
  }

  Token* t = tokens->data[pos];
  if (t->ty == TK_NUM) {
    DEBUG("TK_NUM Found at position(%d) = %d", pos, t->val);
    pos++;
    return new_node_num(t->val);
  }

  error("数値でも開きカッコでもないトークンです: %s", t->input);
}

Node* mul() {
  DEBUG("Entry mul");
  Node* node = term();

  for (;;) {
    if (consume('*')) {
      DEBUG("'*' Found");
      node = new_node('*', node, term());
    } else if (consume('/')) {
      DEBUG("'/' Found");
      node = new_node('/', node, term());
    } else {
      DEBUG("return mul");
      return node;
    }
  }
}

Node* add() {
  DEBUG("Entry add");
  Node* node = mul();

  for (;;) {
    if (consume('+')) {
      DEBUG("'+' Found");
      node = new_node('+', node, mul());
    } else if (consume('-')) {
      DEBUG("'-' Found");
      node = new_node('-', node, mul());
    } else {
      DEBUG("return add");
      return node;
    }
  }
}

void gen(Node* node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
    break;
  }

  printf("  push rax\n");
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
void DEBUG(char* fmt, ...) {
  /*
  va_list ap;
  fprintf(stderr, "DEBUG: ");
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  return;
  */
}

// pが指している文字列をトークンに分割してtokensに保存する
Vector* tokenize(char* p) {
  Vector* v = new_vector();
  
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' ||
	*p == '*' || *p == '/' ||
	*p == '(' || *p == ')') {
      add_token(v, *p, p);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      Token* t = add_token(v, TK_NUM, p);
      t->val = strtol(p, &p, 10);
      continue;
    }

    error("トークナイズできません: %s", p);
    exit(1);
  }

  add_token(v, TK_EOF, p);

  DEBUG("tokens length %d",v->len);

  pos = 0;
  return v;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  char* test = "-test";
  if (strcmp(argv[1], test) == 0) {
    runtest();
    return 0;
  }

  // トークナイズしてパースする
  tokens = tokenize(argv[1]);
  Node* node = add();
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをraxにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
