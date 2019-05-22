#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

Map* global_vars;
Vector* tokens;
int pos = 0;
Node* code[100];
int debug_flg = 0;

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
  printf(".global main\n");
  printf("main:\n");

  int offset = roundup((global_vars->keys->len + 1) * 8 ,16);
  // プロローグ
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", offset);

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
