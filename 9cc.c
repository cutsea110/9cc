#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
      TK_NUM = 256, // 整数トークン
      TK_IDENT,     // 識別子
      TK_RETURN,    // return
      TK_EQ,        // ==
      TK_NE,        // !=
      TK_GE,        // >=
      TK_GT,        // >
      TK_LE,        // <=
      TK_LT,        // <
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
  char* name;       // tyがTK_IDENTの場合その名前
  char* input;      // トークン文字列(エラーメッセージ用)
} Token;

typedef struct Node {
  int ty;           // 演算子かND_NUM,ND_IDENT
  struct Node* lhs; // 左辺
  struct Node* rhs; // 右辺
  int val;          // tyがND_NUMの場合その数値
  char* name;       // ND_IDENTの場合のみ使う
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
Map* new_map();
void map_put(Map*, char*, void*);
void* map_get(Map*, char*);

int is_alnum(char);
Node* assign();
Node* stmt();
void program();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* term();
int consume(int);
void error(char*, ...);
void DEBUG(char*, ...);
void DUMP_TOKENS();
Node* new_node(int, Node*, Node*);
Node* new_node_num(int);
void gen_lval(Node*);
void gen(Node*);

int expect(int, int, int);
void runtest();

int is_alnum(char c) {
  return isalpha(c) || isdigit(c) || c == '_';
}

Map* new_map() {
  Map* map = malloc(sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  return map;
}

void map_put(Map* map, char* key, void* val) {
  DEBUG("Entry map_put for \"%s\"", key);
  vec_push(map->keys, key);
  vec_push(map->vals, val);
  DEBUG("map size is %d", map->keys->len);
}

void* map_get(Map* map, char* key) {
  DEBUG("Entry map_get");
  for (int i = map->keys->len - 1; i >= 0; i--)
    if (strcmp(map->keys->data[i], key) == 0) {
      DEBUG("\"%s\" Found at %d", key, i);
      return map->vals->data[i];
    }
  DEBUG("\"%s\" NOT Found", key);
  return NULL;
}

int map_exists(Map* map, char* key) {
  DEBUG("Entry map_exists for \"%s\"", key);
  for (int i = map->keys->len - 1; i >= 0; i--) {
    if (strcmp(map->keys->data[i], key) == 0) {
      DEBUG("\"%s\" Found at %d", key, i);
      return i;
    }
  }
  DEBUG("\"%s\" NOT Found", key);
  return -1;
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
Map* variables;
int debug_flg = 0;
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

Node* new_node_ident(char* name) {
  Node* node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  DEBUG("return new_node_ident");
  return node;
}

Node* assign() {
  DEBUG("Entry assign");
  Node* node = equality();
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

Node* equality() {
  DEBUG("Entry equality");
  Node* node = relational();
  if (consume(TK_EQ)) {
    DEBUG("\"==\" Found");
    node = new_node(TK_EQ, node, relational());
  } else if (consume(TK_NE)) {
    DEBUG("\"!=\" Found");
    node = new_node(TK_NE, node, relational());
  } else {
    DEBUG("return equality");
    return node;
  }
}

Node* relational() {
  DEBUG("Entry relational");
  Node* node = add();
  if (consume(TK_GE)) {
    DEBUG("\">=\" Found");
    node = new_node(TK_GE, node, add());
  } else if (consume(TK_GT)) {
    DEBUG("\">\" Found");
    node = new_node(TK_GT, node, add());
  } else if (consume(TK_LE)) {
    DEBUG("\"<=\" Found");
    node = new_node(TK_LE, node, add());
  } else if (consume(TK_LT)) {
    DEBUG("\"<\" Found");
    node = new_node(TK_LT, node, add());
  } else {
    DEBUG("return relational");
    return node;
  }
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

Node* unary() {
  DEBUG("Entry unary");
  if (consume('+')) {
    DEBUG("'+' Found");
    return term();
  }
  if (consume('-')) {
    DEBUG("'-' Found");
    return new_node('-', new_node_num(0), term());
  }
  return term();
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
    return new_node_ident(t->name);
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
  Node* node = unary();

  for (;;) {
    if (consume('*')) {
      DEBUG("'*' Found");
      node = new_node('*', node, unary());
    } else if (consume('/')) {
      DEBUG("'/' Found");
      node = new_node('/', node, unary());
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

  int offset = map_exists(variables, node->name);
  if (offset == -1) {
    map_put(variables, node->name, (void*)NULL);
    offset = map_exists(variables, node->name);
  }
  DEBUG("\"%s\" Found with offset(%d)", node->name, offset);
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", (offset + 1) * 8);
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
  case TK_EQ:
    printf("  cmp rdi, rax\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_NE:
    printf("  cmp rdi, rax\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_GE:
    printf("  cmp rdi, rax\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_GT:
    printf("  cmp rdi, rax\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case TK_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
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
void DUMP_TOKENS() {
  for (int i = 0; i < tokens->len; i++) {
    Token* t = tokens->data[i];
    switch(t->ty) {
    case TK_NUM:
      fprintf(stderr, "tokens[%d]: %d\n",i, t->val);
      break;
    case TK_IDENT:
      fprintf(stderr, "tokens[%d]: \"%s\"\n",i, t->name);
      break;
    case TK_EQ:
      fprintf(stderr, "tokens[%d]: TK_EQ\n", i);
      break;
    case TK_NE:
      fprintf(stderr, "tokens[%d]: TK_NE\n", i);
      break;
    case TK_GE:
      fprintf(stderr, "tokens[%d]: TK_GE\n", i);
      break;
    case TK_GT: 
      fprintf(stderr, "tokens[%d]: TK_GT\n", i);
      break;
    case TK_LE:
      fprintf(stderr, "tokens[%d]: TK_LE\n", i);
      break;
    case TK_LT:
      fprintf(stderr, "tokens[%d]: TK_LT\n", i);
      break;
    case TK_RETURN:
      fprintf(stderr, "tokens[%d]: TK_RETURN\n", i);
      break;
    case TK_EOF:
      fprintf(stderr, "tokens[%d]: TK_EOF\n", i);
      break;
    default:
      fprintf(stderr, "tokens[%d]: %c\n",i, t->ty);
      break;
    }
  }
}
void DEBUG(char* fmt, ...) {
  if (debug_flg) {
    va_list ap;
    fprintf(stderr, "DEBUG: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }
  return;
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

    if (strncmp(p, "==", 2) == 0) {
      add_token(v, TK_EQ, p);
      p += 2;
      continue;
    }
    
    if (strncmp(p, "!=", 2) == 0) {
      add_token(v, TK_NE, p);
      p += 2;
      continue;
    }

    if (strncmp(p, ">=", 2) == 0) {
      add_token(v, TK_GE, p);
      p += 2;
      continue;
    }
    
    if (*p == '>') {
      add_token(v, TK_GT, p);
      p++;
      continue;
    }
    
    if (strncmp(p, "<=", 2) == 0) {
      add_token(v, TK_LE, p);
      p += 2;
      continue;
    }
    
    if (*p == '<') {
      add_token(v, TK_LT, p);
      p++;
      continue;
    }
    
    if (isalpha(*p) || *p == '_') {
      int len = 1;
      while (is_alnum(p[len]))
	len++;
      char* name = strndup(p, len);
      int ty = (intptr_t)map_get(variables, name);
      if (!ty) {
	ty = TK_IDENT;
	DEBUG("\"%s\" Found", name);
      }
      Token* t = add_token(v, ty, p);
      t->name = name;
      p += len;
      continue;
    }
    
    if (strchr("+-*/()=;", *p)) {
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
  char* test = "-test";
  char* debug = "-d";
  char* prog_code = argv[1];
  
  if (argc == 3 && strcmp(argv[1], debug) == 0) {
    debug_flg = 1;
    prog_code = argv[2];
  } else if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  } else if (strcmp(argv[1], test) == 0) {
    runtest();
    return 0;
  }

  variables = new_map();

  // トークナイズしてパースする
  // 結果はcodeに保存される
  tokens = tokenize(prog_code);
  if (debug_flg)
    DUMP_TOKENS();
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
