#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

int label_count = 0;
char* regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

extern void gen_lval(Node* node) {
  if (node->ty == ND_IDENT) {
    Map* m = vars_map();
    int offset = map_exists(m, node->name);
    if (offset == -1) {
      error("\"%s\" NOT Found in symbol table", node->name);
    }
    DEBUG("\"%s\" Found with offset(%d)", node->name, offset);
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", (offset + 1) * 8);
    printf("  push rax\n");
  } else if (node->ty == ND_DEREF) {
    gen(node->lhs);
  } else
    error("代入の左辺値が変数ではありません");
}

extern void gen(Node* node) {

  if (node->ty == ND_VARDEF) {
    DEBUG("ND_VARDEF Found");
    return;
  }

  if (node->ty == ND_DEREF) {
    DEBUG("ND_DEREF Found");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_REF) {
    DEBUG("ND_REF Found");
    gen_lval(node->lhs);
    return;
  }
  
  if (node->ty == ND_RETURN) {
    DEBUG("ND_RETURN Found");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  if (node->ty == ND_IF) {
    DEBUG("ND_IF Found");
    if (node->rhs->ty == ND_ELSE) {
      DEBUG("ND_ELSE Found");
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

  if (node->ty == ND_WHILE) {
    DEBUG("ND_WHILE Found");
    int lcnt = ++label_count;
    printf(".Lbegin%04d:\n", lcnt);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%04d\n", lcnt);
    gen(node->rhs);
    printf("  jmp .Lbegin%04d\n", lcnt);
    printf(".Lend%04d:\n", lcnt);
    return;
  }

  if (node->ty == ND_FOR) {
    DEBUG("ND_FOR Found");
    int lcnt = ++label_count;
    Node* cond = node->lhs;
    if (cond->lhs->ty != ND_NOP) {
      DEBUG("Initial expr Found");
      gen(cond->lhs);
    }
    printf(".Lbegin%04d:\n", lcnt);
    if (cond->rhs->lhs->ty != ND_NOP) {
      DEBUG("Terminal expr Found");
      gen(cond->rhs->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%04d\n", lcnt);
    }
    gen(node->rhs);
    if (cond->rhs->rhs->ty != ND_NOP) {
      DEBUG("Step expr Found");
      gen(cond->rhs->rhs);
    }
    printf("  jmp .Lbegin%04d\n", lcnt);
    printf(".Lend%04d:\n", lcnt);
    return;
  }

  if (node->ty == ND_BLOCK) {
    DEBUG("ND_BLOCK Found");
    Vector* blk = node->blk;
    for (int j = 0; j < blk->len; j++) {
      Node* st = blk->data[j];
      gen(st);
      printf("  pop rax\n");
    }
    return;
  }

  if (node->ty == ND_FUNDEF) {
    current_vars = node->local_vars;
    printf(".global %s\n", node->name);
    printf("%s:\n", node->name);

    int offset = roundup((node->local_vars->keys->len + 1) * 8 ,16);
    // プロローグ
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    for (int j = 0; j < node->arg_num; j++) {
      printf("  push %s\n", regs[j]);
      offset -= 8;
    }
    if (offset > 0)
      printf("  sub rsp, %d\n", offset);
    
    for (int j = 0; node->blk->data[j]; j++) {
      gen((Node*)node->blk->data[j]);
    }
    
    return;
  }

  if (node->ty == ND_FUNCALL) {
    DEBUG("ND_FUNCALL Found");
    Vector* args = node->rhs->blk;
    for (int j = 0; j < args->len; j++) {
      Node* exp = args->data[j];
      gen(exp);
      printf("  pop %s\n", regs[j]);
    }
    Node* fun = node->lhs;
    printf("  call %s\n", fun->name);
    printf("  push rax\n");
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
  case ND_EQ:
    printf("  cmp rdi, rax\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rdi, rax\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_GE:
    error("ND_GE is replaced to ND_LE");
    break;
  case ND_GT:
    error("ND_GT is replaced to ND_LT");
    break;
  }

  printf("  push rax\n");
}
