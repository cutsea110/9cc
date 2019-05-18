#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int label_count = 0;

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

  if (node->ty == ND_IF) {
    if (node->rhs->ty == ND_ELSE) {
      int lcnt = ++label_count;
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lelse%04d\n", lcnt);
      gen(node->rhs->lhs);
      printf("  jmp .Lend%04d\n", lcnt);
      printf(".Lelse%04d:\n", lcnt);
      gen(node->rhs->rhs);
      printf(".Lend%04d:\n", lcnt);
      return;
    } else {
      int lcnt = ++label_count;
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%04d\n", lcnt);
      gen(node->rhs);
      printf(".Lend%04d:\n", lcnt);
      return;
    }
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
