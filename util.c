#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "9cc.h"

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
    case TK_EOF:
      fprintf(stderr, "tokens[%d]: TK_EOF\n", i);
      break;
    default:
      fprintf(stderr, "tokens[%d]: %c\n",i, t->ty);
      break;
    }
  }
}
