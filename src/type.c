#include "9cc.h"

Type *new_type(TypeKind kind) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = kind;
  return type;
}

Type *char_type() {
  return new_type(TY_CHAR);
}

Type *int_type() {
  return new_type(TY_INT);
}

Type *pointer_to(Type *base) {
  Type *type = new_type(TY_PTR);
  type->base = base;
  return type;
}

Type *array_type(Type *base, int size) {
  Type *type = new_type(TY_ARRAY);
  type->base = base;
  type->array_size = size;
  return type;
}

int size_of(Type *type) {
  switch (type->kind) {
  case TY_CHAR:
    return 1;
  case TY_INT:
    return 4;
  case TY_PTR:
    return 8;
  case TY_ARRAY:
    return size_of(type->base) * type->array_size;
  }
  error("不明な型です");
}

int align_of(Type *type) {
  switch (type->kind) {
  case TY_CHAR:
    return 1;
  case TY_INT:
    return 4;
  case TY_PTR:
    return 8;
  case TY_ARRAY:
    return size_of(type->base);
  }
  error("不明な型です");
}

void visit(Node *node) {
  if (!node)
    return;

  visit(node->lhs);
  visit(node->rhs);
  visit(node->cond);
  visit(node->then);
  visit(node->els);
  visit(node->init);
  visit(node->inc);

  for (Node *n = node->block; n; n = n->next)
    visit(n);
  for (Node *n = node->args; n; n = n->next)
    visit(n);

  switch (node->kind) {
  case ND_MUL:
  case ND_DIV:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_FUNCALL:
  case ND_NUM:
    node->type = int_type();
    return;
  case ND_VAR:
    node->type = node->var->type;
    return;
  case ND_ADD:
    if (node->rhs->type->base) {
      Node *tmp = node->lhs;
      node->lhs = node->rhs;
      node->rhs = tmp;
    }
    if (node->rhs->type->base)
      error_tok(node->tok, "無効なポインタ演算オペランドです");
    node->type = node->lhs->type;
    return;
  case ND_SUB:
    if (node->rhs->type->base)
      error_tok(node->tok, "無効なポインタ演算オペランドです");
    node->type = node->lhs->type;
    return;
  case ND_ASSIGN:
    node->type = node->lhs->type;
    return;
  case ND_ADDR:
    if (node->lhs->type->kind == TY_ARRAY)
      node->type = pointer_to(node->lhs->type->base);
    else
      node->type = pointer_to(node->lhs->type);
    return;
  case ND_DEREF:
    if (!node->lhs->type->base)
      error_tok(node->tok, "無効なポインタ参照です");
    node->type = node->lhs->type->base;
    return;
  case ND_SIZEOF:
    node->kind = ND_NUM;
    node->type = int_type();
    node->val = size_of(node->lhs->type);
    node->lhs = NULL;
    return;
  case ND_STMT_EXPR:
    Node *last = node->block;
    while (last->next)
      last = last->next;
    node->type = last->type;
    return;
  }
}

void add_type(Program *prog) {
  for (Function *fn = prog->fns; fn; fn = fn->next)
    for (Node *node = fn->node; node; node = node->next)
      visit(node);
}
