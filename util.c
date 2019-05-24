#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

int roundup(int x, int align) {
  return (x + align - 1) & ~(align - 1);
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
  if (debug_flg) {
    va_list ap;
    fprintf(stderr, "DEBUG: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }
  return;
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
    case TK_IF:
      fprintf(stderr, "tokens[%d]: TK_IF\n", i);
      break;
    case TK_ELSE:
      fprintf(stderr, "tokens[%d]: TK_ELSE\n", i);
      break;
    case TK_FOR:
      fprintf(stderr, "tokens[%d]: TK_FOR\n", i);
      break;
    case TK_WHILE:
      fprintf(stderr, "tokens[%d]: TK_WHILE\n", i);
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

void dump_code(int i, Node* node, int level) {
  if (level == 0) {
    fprintf(stderr, "=======================\n");
    fprintf(stderr, "  code[%d]\n", i);
    fprintf(stderr, "=======================\n");
  }
  // インデントする
  for (int n = 0; n < level; n++)
    fprintf(stderr, " ");
  
  switch (node->ty) {
  case ND_NUM:
    fprintf(stderr, "ND_NUM: %d\n", node->val);
    break;
  case ND_IDENT:
    fprintf(stderr, "ND_IDENT: \"%s\"\n", node->name);
    break;
  case ND_RETURN:
    fprintf(stderr, "ND_RETURN:\n");
    dump_code(i, node->lhs, level+1);
    break;
  case ND_IF:
    fprintf(stderr, "ND_IF:", i);
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_ELSE:
    fprintf(stderr, "ND_ELSE:", i);
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_WHILE:
    fprintf(stderr, "ND_WHILE:");
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_FOR:
    fprintf(stderr, "ND_FOR:");
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_BLOCK:
    fprintf(stderr, "ND_BLOCK:");
    Vector* blk = node->blk;
    for (int j = 0; j < blk->len; j++) {
      Node* st = blk->data[j];
      dump_code(i, st, level+1);
    }
    break;
  case ND_FUNDECL:
    fprintf(stderr, "ND_FUNDECL: %s\n", node->name);
    // TODO
    break;
  case ND_FUNCALL:
    fprintf(stderr, "ND_FUNCALL:\n");
    dump_code(i, node->lhs, level+1);
    break;
  case ND_EQ:
    fprintf(stderr, "ND_EQ:\n");
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_NE:
    fprintf(stderr, "ND_NE:\n");
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_LE:
    fprintf(stderr, "ND_LE:\n");
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_LT:
    fprintf(stderr, "ND_LT:\n");
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_GE:
    fprintf(stderr, "ND_GE:\n");
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_GT:
    fprintf(stderr, "ND_GT:\n");
    dump_code(i, node->lhs, level+1);
    dump_code(i, node->rhs, level+1);
    break;
  case ND_NOP:
    fprintf(stderr, "ND_NOP\n");
    break;
  }
}

void DUMP_CODES() {
  for (int i = 0; code[i]; i++) {
    Node* node = code[i];
    dump_code(i, node, 0);
  }
}
