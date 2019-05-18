#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int is_alnum(char c);
int consume(int ty);

Vector* tokenize(char* p);

void program();
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* term();

Node* new_node(int ty, Node* lhs, Node* rhs);
Node* new_node_num(int val);
Node* new_node_ident(char* name);

int is_alnum(char c) {
  return isalpha(c) || isdigit(c) || c == '_';
}

int consume(int ty) {
  Token* t = tokens->data[pos];
  if (t->ty != ty)
    return 0;
  pos++;
  return 1;
}

Token* add_token(Vector* v, int ty, char* p) {
  Token* t = malloc(sizeof(Token));
  t->ty = ty;
  t->input = p;
  vec_push(v, t);
  return t;
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

    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      add_token(v, TK_WHILE, p);
      p += 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      add_token(v, TK_FOR, p);
      p += 3;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      add_token(v, TK_IF, p);
      p += 2;
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

void program() {
  DEBUG("Entry program");
  int i = 0;
  while (((Token*)tokens->data[pos])->ty != TK_EOF) {
    DEBUG("add stmt at code[%d]", i);
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node* stmt() {
  DEBUG("Entry stmt");
  Node* node;
  
  if (consume(TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }
  
  if (!consume(';')) {
    DEBUG("';' NOT Found");
    Token* t = tokens->data[pos];
    error("';'ではないトークンです: %s", t->input);
  }
  DEBUG("';' Found");
  return node;
}

Node* expr() {
  return assign();
}

Node* assign() {
  DEBUG("Entry assign");
  Node* node = equality();
  while(consume('=')) {
    node = new_node('=', node, assign());
  }
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
