#include "9cc.h"

int labelseq = 0;
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->var->offset);
    printf("  push rax\n");
    return;
  case ND_DEREF:
    gen(node->lhs);
    return;
  }
  error("代入の左辺値が変数ではありません");
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

  int seq = labelseq++;
  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_VAR:
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
  case ND_WHILE:
    printf(".Lbegin%d:\n", seq);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", seq);
    gen(node->then);
    printf("  jmp .Lbegin%d\n", seq);
    printf(".Lend%d:\n", seq);
    return;
  case ND_FOR:
    if (node->init)
      gen(node->init);
    printf(".Lbegin%d:\n", seq);
    if (node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%d\n", seq);
    }
    gen(node->then);
    if (node->inc)
      gen(node->inc);
    printf("  jmp .Lbegin%d\n", seq);
    printf(".Lend%d:\n", seq);
    return;
  case ND_BLOCK:
    // node->block[i] != NULLでなければループしてgen
    for (Node *block = node->block; block; block = block->next) {
      gen(block);
      if (block->kind != ND_RETURN) {
        printf("  pop rax\n");
      }
    }
    return;
  case ND_FUNCALL:
    int arg_count = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      gen(arg);
      arg_count++;
    }

    for (int i = arg_count - 1; i >= 0; i--)
      printf("  pop %s\n", argreg[i]);

    int seq = labelseq++;
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n");
    printf("  jnz .Lcall%d\n", seq);
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  jmp .Lend%d\n", seq);
    printf(".Lcall%d:\n", seq);
    printf("  sub rsp, 8\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", node->funcname);
    printf("  add rsp, 8\n");
    printf(".Lend%d:\n", seq);
    printf("  push rax\n");
    return;
  case ND_ADDR:
    gen_lval(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
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

void codegen(Function *prog) {
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");

  for (Function *fn = prog; fn; fn = fn->next) {
    printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);

    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);

    int i = 0;
    for (VarList *vl = fn->params; vl; vl = vl->next) {
      Var *var = vl->var;
      printf("  mov [rbp-%d], %s\n", vl->var->offset, argreg[i++]);
    }

    // 先頭の式から順にコード生成
    for (Node *node = fn->node; node; node = node->next) {
      /* print_ast(node, 0); */
      gen(node);
    }

    // エピローグ
    // 最後の式の結果がRAX残っているので、それが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }

  printf(".section .note.GNU-stack,\"\",@progbits\n"); // 警告を消すため
}

void print_ast(Node *node, int depth) {
  for (int i = 0; i < depth; i++)
    printf("  ");
  switch (node->kind) {
  case ND_NUM:
    printf("NUM: %d\n", node->val);
    return;
  case ND_VAR:
    printf("VAR: offset=%d\n", node->var->offset);
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
  case ND_RETURN:
    printf("RETURN\n");
    break;
  case ND_IF:
    printf("IF\n");
    break;
  case ND_WHILE:
    printf("WHILE\n");
    break;
  case ND_FOR:
    printf("FOR\n");
    break;
  case ND_BLOCK:
    printf("BLOCK\n");
    break;
  case ND_ADDR:
    printf("ADDR\n");
    break;
  case ND_DEREF:
    printf("DEREF\n");
    break;
  }

  // ND_NUM や ND_VAR のように左右の子を持たないノードでは再帰しない
  if (node->lhs)
    print_ast(node->lhs, depth + 1);
  if (node->rhs)
    print_ast(node->rhs, depth + 1);
  if (node->cond)
    print_ast(node->cond, depth + 1);
  if (node->then)
    print_ast(node->then, depth + 1);
  if (node->els)
    print_ast(node->els, depth + 1);
  if (node->init)
    print_ast(node->init, depth + 1);
  if (node->inc)
    print_ast(node->inc, depth + 1);
}
