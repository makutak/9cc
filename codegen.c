#include "9cc.h"

int labelseq = 0;
char *funcname;
static char *argreg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argreg4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argreg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
  switch (node->kind) {
  case ND_VAR: {
    Var *var = node->var;
    if (var->is_local) {
      printf("  lea rax, [rbp-%d]\n", node->var->offset);
      printf("  push rax\n");
    } else {
      printf("  push offset %s\n", var->name);
    }
    return;
  }
  case ND_DEREF:
    gen(node->lhs);
    return;
  }
  error_tok(node->tok, "代入の左辺値が変数ではありません");
}

void gen(Node *node) {
  if (node->kind == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  jmp .Lreturn.%s\n", funcname);
    return;
  }

  int seq = labelseq++;
  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_VAR:
    gen_lval(node);
    if (node->type->kind != TY_ARRAY) {
      printf("  pop rax\n");
      if (node->type->kind == TY_CHAR)
        printf("  movzx rax, BYTE PTR [rax]\n");
      else if (node->type->kind == TY_INT)
        printf("  movsxd rax, DWORD PTR [rax]\n");
      else
        printf("  mov rax, [rax]\n");
      printf("  push rax\n");
    }
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    if (node->type->kind == TY_CHAR)
      printf("  mov [rax], dil\n");
    else if (node->type->kind == TY_INT)
      printf("  mov [rax], edi\n");
    else
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
      printf("  pop %s\n", argreg8[i]);

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
    if (node->type->kind != TY_ARRAY) {
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
    }
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    if (node->type->base)
      printf("  imul rdi, %d\n", size_of(node->type->base));
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    if (node->type->base)
      printf("  imul rdi, %d\n", size_of(node->type->base));
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
    printf("  movzx rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzx rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzx rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzx rax, al\n");
    break;
  }

  printf("  push rax\n");
}

void emit_data(Program *prog) {
  printf(".data\n");

  for (VarList *vl = prog->globals; vl; vl = vl->next) {
    Var *var = vl->var;
    printf("%s:\n", var->name);
    printf("  .zero %d\n", size_of(var->type));
  }
}

void emit_text(Program *prog) {
  printf(".text\n");

  for (Function *fn = prog->fns; fn; fn = fn->next) {
    printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);
    funcname = fn->name;

    // プロローグ
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);

    int i = 0;
    for (VarList *vl = fn->params; vl; vl = vl->next) {
      Var *var = vl->var;
      if (var->type->kind == TY_CHAR)
        printf("  mov BYTE PTR [rbp-%d], %s\n", vl->var->offset, argreg1[i++]);
      else if (var->type->kind == TY_INT)
        printf("  mov DWORD PTR [rbp-%d], %s\n", vl->var->offset, argreg4[i++]);
      else
        printf("  mov QWORD PTR [rbp-%d], %s\n", vl->var->offset, argreg8[i++]);
    }

    // 先頭の式から順にコード生成
    for (Node *node = fn->node; node; node = node->next) {
      /* print_ast(node, 0); */
      gen(node);
    }

    // エピローグ
    printf(".Lreturn.%s:\n", funcname);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}

void codegen(Program *prog) {
  printf(".intel_syntax noprefix\n");
  emit_data(prog);
  emit_text(prog);
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
