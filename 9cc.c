#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

extern Map* global_vars;
extern Map* current_vars;

extern Map* vars_map() {
  return current_vars != NULL ? current_vars : global_vars;
}

extern Vector* tokens;
extern int pos;
extern Node* code[100];
extern int debug_flg;

int main(int argc, char** argv) {
  pos = 0;
  debug_flg = 0;
  
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

  global_vars = new_map();

  // トークナイズしてパースする
  // 結果はcodeに保存される
  tokens = tokenize(prog_code);
  if (debug_flg)
    DUMP_TOKENS();

  program();

  if (debug_flg)
    DUMP_CODES();
  
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);
    // 式の評価結果としてスタックに一つの値が残っているはずなので
    // スタックが溢れないようにポップしておく
    printf("  pop rax\n");
  }

  return 0;
}
