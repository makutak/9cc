#include "9cc.h"

int labelseq = 0;

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node->kind == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;

  case ND_IF:
    int seq = labelseq++;
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lelse%d\n", seq);
    gen(node->then);
    printf("  jmp .Lend%d\n", seq);
    printf(".Lelse%d:\n", seq);
    if (node->els)
      gen(node->els);
    printf(".Lend%d:\n", seq);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

void codegen(Node *node) {
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードしてから関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");

  printf(".section .note.GNU-stack,\"\",@progbits\n"); // 警告を消すため
}

void print_ast(Node *node, int depth) {
  for (int i = 0; i < depth; i++)
    printf("  ");
  switch (node->kind) {
  case ND_NUM:
    printf("NUM: %d\n", node->val);
    return;
  case ND_LVAR:
    printf("LVAR: offset=%d\n", node->offset);
    return;
  case ND_ASSIGN:
    printf("ASSIGN\n");
    break;
  case ND_ADD:
    printf("ADD\n");
    break;
  case ND_SUB:
    printf("SUB\n");
    break;
  case ND_MUL:
    printf("MUL\n");
    break;
  case ND_DIV:
    printf("DIV\n");
    break;
  case ND_EQ:
    printf("EQ\n");
    break;
  case ND_NE:
    printf("NE\n");
    break;
  case ND_LT:
    printf("LT\n");
    break;
  case ND_LE:
    printf("LE\n");
    break;
  }

  // ND_NUM や ND_LVAR のように左右の子を持たないノードでは再帰しない
  if (node->lhs)
    print_ast(node->lhs, depth + 1);
  if (node->rhs)
    print_ast(node->rhs, depth + 1);
}
