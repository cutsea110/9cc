#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int is_alnum(char c);
int consume(int ty);

Vector* tokenize(char* p);

void program();
Node* decl();
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
  if (t->ty != ty) {
    return 0;
  } else if (t->ty == TK_IDENT) {
    Map* m = vars_map();
    if (map_get(m, t->name) == NULL) {
      map_put(m, t->name, (void*)NULL);
    }
    pos++;
    return 1;
  } else {
    pos++;
    return 1;
  }
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

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      add_token(v, TK_ELSE, p);
      p += 4;
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
      int ty = (intptr_t)map_get(vars_map(), name);
      if (!ty) {
	ty = TK_IDENT;
	DEBUG("\"%s\" Found", name);
      }
      Token* t = add_token(v, ty, p);
      t->name = name;
      p += len;
      continue;
    }
    
    if (strchr("+-*/(){}=,;", *p)) {
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
    code[i++] = decl();
  }
  code[i] = NULL;
}

Node* decl() {
  Node* node = malloc(sizeof(Node));
  node->ty = ND_FUNDEF;
  
  Token* t = tokens->data[pos];
  if (!consume(TK_IDENT))
    error("関数名でないトークンです: %s", t->input);

  node->name = t->name;

  if (!consume('('))
    error("'('でないトークンです: %s", t->input);

  current_vars = new_map();

  int c = 0;
  while (consume(TK_IDENT)) {
    c++;
    if (consume(',')) {
      continue;
    } else if (consume(')')) {
      break;
    } else {
      error("','でも')'でもないトークンです: %s", t->input);
    }
  }

  node->arg_num = c;
  
  t = tokens->data[pos];
  if (!consume('{'))
    error("'{'でないトークンです: %s", t->input);

  Vector* vec = new_vector();
  while (!consume('}')) {
    vec_push(vec, stmt());
  }
  node->local_vars = current_vars;
  current_vars = NULL;
  
  node->blk = vec;
  return node;
}

Node* stmt() {
  DEBUG("Entry stmt");
  Node* node;
  
  if (consume(TK_RETURN)) {
    DEBUG("\"return\" found");
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = expr();
  } else if (consume(TK_IF)) {
    DEBUG("\"if\" found");
    node = malloc(sizeof(Node));
    node->ty = ND_IF;
    if (consume('(')) {
      node->lhs = expr();
      if (consume(')')) {
	Node* stt = stmt();
	if (consume(TK_ELSE)) {
	  DEBUG("\"else\" found");
	  Node* ste = malloc(sizeof(Node));
	  ste->ty = ND_ELSE;
	  ste->lhs = stt;
	  ste->rhs = stmt();
	  node->rhs = ste;
	} else {
	  node->rhs = stt;
	}
	return node;
      } else {
	DEBUG("')' NOT Found");
	Token* t = tokens->data[pos];
	error("')'ではないトークンです: %s", t->input);
      }
    } else {
      DEBUG("'(' NOT Found");
      Token* t = tokens->data[pos];
      error("'('ではないトークンです: %s", t->input);
    }
  } else if (consume(TK_WHILE)) {
    DEBUG("\"while\" found");
    node = malloc(sizeof(Node));
    node->ty = ND_WHILE;
    if (consume('(')) {
      node->lhs = expr();
      if (consume(')')) {
	node->rhs = stmt();
	return node;
      } else {
	DEBUG("')' NOT Found");
	Token* t = tokens->data[pos];
	error("')'ではないトークンです: %s", t->input);
      }
    } else {
      DEBUG("'(' NOT Found");
      Token* t = tokens->data[pos];
      error("'('ではないトークンです: %s", t->input);
    }
  } else if (consume(TK_FOR)) {
    DEBUG("\"for\" found");
    node = malloc(sizeof(Node));
    node->ty = ND_FOR;

    Node* nop = malloc(sizeof(Node));
    nop->ty = ND_NOP;
    
    if (consume('(')) {
      Node* cond = malloc(sizeof(Node));
      cond->rhs = malloc(sizeof(Node));

      if (consume(';')) {
	cond->lhs = nop;
      } else {
	cond->lhs = expr();
	if (!consume(';')) {
	  DEBUG("';' NOT Found");
	  Token* t = tokens->data[pos];
	  error("';'ではないトークンです: %s", t->input);
	}
      }
      
      if (consume(';')) {
	cond->rhs->lhs = nop;
      } else {
	cond->rhs->lhs = expr();
	if (!consume(';')) {
	  DEBUG("';' NOT Found");
	  Token* t = tokens->data[pos];
	  error("';'ではないトークンです: %s", t->input);
	}
      }
      
      if (consume(')')) {
	cond->rhs->rhs = nop;
      } else {
	cond->rhs->rhs = expr();
	if (!consume(')')) {
	  DEBUG("')' NOT Found");
	  Token* t = tokens->data[pos];
	  error("')'ではないトークンです: %s", t->input);
	}
      }

      node->lhs = cond;
      node->rhs = stmt();
      return node;
    } else {
      DEBUG("'(' NOT Found");
      Token* t = tokens->data[pos];
      error("'('ではないトークンです: %s", t->input);
    }
  } else if (consume('{')) {
    DEBUG("'{' found, block start");
    Node* node = malloc(sizeof(Node));
    node->ty = ND_BLOCK;
    node->blk = new_vector();
    while (!consume('}')) {
      Node* st = stmt();
      vec_push(node->blk, (void*)st);
    }
    DEBUG("'}' found, block end");
    return node;
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
    node = new_node(ND_EQ, node, relational());
  } else if (consume(TK_NE)) {
    DEBUG("\"!=\" Found");
    node = new_node(ND_NE, node, relational());
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
    node = new_node(ND_LE, add(), node);
  } else if (consume(TK_GT)) {
    DEBUG("\">\" Found");
    node = new_node(ND_LT, add(), node);
  } else if (consume(TK_LE)) {
    DEBUG("\"<=\" Found");
    node = new_node(ND_LE, node, add());
  } else if (consume(TK_LT)) {
    DEBUG("\"<\" Found");
    node = new_node(ND_LT, node, add());
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
    Node* node = expr();
    if (!consume(')')) {
      DEBUG("')' NOT Found");
      Token* t = tokens->data[pos];
      error("開きカッコに対応する閉じカッコがありません: %s", t->input);
    }
    DEBUG("')' Found");
    return node;
  }

  Token* t = tokens->data[pos];
  if (consume(TK_IDENT)) {
    if (consume('(')) {
      int arg_count = 0;
      Node* arg = malloc(sizeof(Node));
      arg->ty = ND_ARGS;
      arg->blk = new_vector();
      while (!consume(')')) {
	vec_push(arg->blk, expr());
	if (consume(',')) {
	  continue;
	} else if (consume(')')) {
	  break;
	} else {
	  error("関数呼び出しにおける引数リストの与え方が正しくありません: %s", t->input);
	}
      }
      Node* node = malloc(sizeof(Node));
      node->ty = ND_FUNCALL;
      node->lhs = new_node_ident(t->name);
      node->rhs = arg;
      return node;
    } else {
      return new_node_ident(t->name);
    }
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
