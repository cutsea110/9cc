#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
      TK_NUM = 256, // 整数トークン
      TK_IDENT,     // 識別子
      TK_RETURN,    // return
      TK_EOF,       // 入力の終端を表すトークン
};

enum {
      ND_NUM = 256, // 整数のノードの型
      ND_IDENT,     // 識別子
      ND_RETURN,    // return
};

typedef struct {
  int ty;           // トークンの型
  int val;          // tyがTK_NUMの場合その数値
  char* input;      // トークン文字列(エラーメッセージ用)
} Token;

typedef struct Node {
  int ty;           // 演算子かND_NUM,ND_IDENT
  struct Node* lhs; // 左辺
  struct Node* rhs; // 右辺
  int val;          // tyがND_NUMの場合その数値
  char name;        // ND_IDENTの場合のみ使う
} Node;

typedef struct {
  void** data;      // データ本体
  int capacity;     // バッファの大きさ(data[0]~data[capacity-1]がバッファ領域)
  int len;          // ベクタに追加済みの要素数
} Vector;

typedef struct {
  Vector* keys;
  Vector* vals;
} Map;

Vector* new_vector();
void vec_push(Vector*, void*);
Token* add_token(Vector*, int, char*);

Node* assign();
Node* stmt();
void program();
Node* add();
Node* mul();
Node* term();
int consume(int);
void error(char*, ...);
void DEBUG(char*, ...);
Node* new_node(int, Node*, Node*);
Node* new_node_num(int);
void gen_lval(Node*);
void gen(Node*);

int expect(int, int, int);
void runtest();

int is_alnum(char c) {
  return
    ('a' <= c && c <= 'z') ||
    ('A' <= c && c <= 'Z') ||
    ('0' <= c && c <= '9') ||
    (c == '_');
}

Map* new_map() {
  Map* map = malloc(sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  return map;
}

void map_put(Map* map, char* key, void* val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void* map_get(Map* map, char* key) {
  for (int i = map->keys->len - 1; i >= 0; i--)
    if (strcmp(map->keys->data[i], key) == 0)
      return map->vals->data[i];
  return NULL;
}

int expect(int line, int expected, int actual) {
  if (expected == actual)
    return 1;
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);
}

void test_vector() {
  Vector* vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++)
    vec_push(vec, (void*)i);

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);
  
  printf("Vector OK\n");
}

void test_map() {
  Map* map = new_map();

  expect(__LINE__, 0, (int)map_get(map, "foo"));

  map_put(map, "foo", (void*)2);
  expect(__LINE__, 2, (int)map_get(map, "foo"));
  
  map_put(map, "bar", (void*)4);
  expect(__LINE__, 4, (int)map_get(map, "bar"));
  expect(__LINE__, 2, (int)map_get(map, "foo"));

  map_put(map, "foo", (void*)6);
  expect(__LINE__, 6, (int)map_get(map, "foo"));
  expect(__LINE__, 4, (int)map_get(map, "bar"));

  printf("Map OK\n");
}

void runtest() {
  test_vector();
  test_map();
  printf("OK\n");
}

Vector* tokens;
int pos = 0;
Node* code[100];

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

Node* new_node_ident(char name) {
  Node* node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  DEBUG("return new_node_ident");
  return node;
}

Node* assign() {
  DEBUG("Entry assign");
  Node* node = add();
  while(consume('=')) {
    node = new_node('=', node, assign());
  }
  return node;
}

Node* stmt() {
  DEBUG("Entry stmt");
  Node* node;
  
  if (consume(TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = assign();
  } else {
    node = assign();
  }
  
  if (!consume(';')) {
    DEBUG("';' NOT Found");
    Token* t = tokens->data[pos];
    error("';'ではないトークンです: %s", t->input);
  }
  DEBUG("';' Found");
  return node;
}

void program() {
  DEBUG("Entry program");
  int i = 0;
  while (((Token*)tokens->data[pos])->ty != TK_EOF) {
    DEBUG("add stmt at code[%d]", i);
    code[i++] = stmt();
  }
  code[i] = NULL;
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
  if (t->ty == TK_IDENT) {
    DEBUG("TK_IDENT Found at position(%d) = %s", pos, t->input);
    pos++;
    return new_node_ident(*(t->input));
  }
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

void gen_lval(Node* node) {
  if (node->ty != ND_IDENT)
    error("代入の左辺値が変数ではありません");
  int offset = ('z' - node->name + 1) * 8;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}

void gen(Node* node) {
  if (node->ty == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
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

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      add_token(v, TK_RETURN, p);
      p += 6;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      Token* t = add_token(v, TK_IDENT, p);
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' ||
	*p == '*' || *p == '/' ||
	*p == '(' || *p == ')' ||
	*p == '=' || *p == ';') {
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
  // 結果はcodeに保存される
  tokens = tokenize(argv[1]);
  program();
  
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // プロローグ
  // 変数26個分の領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n"); // 26 * 8 = 208

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);
    // 式の評価結果としてスタックに一つの値が残っているはずなので
    // スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がraxに残っているはずなのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
