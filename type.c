#include "9cc.h"

Type *int_type() {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TY_INT;
  return type;
}

Type *pointer_to(Type *base) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TY_PTR;
  type->base = base;
  return type;
}

int size_of(Type *type) {
  if (type->kind == TY_INT)
    return 4;
  else
    return 8;
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
    if (node->rhs->type->kind == TY_PTR) {
      Node *tmp = node->lhs;
      node->lhs = node->rhs;
      node->rhs = tmp;
    }
    if (node->rhs->type->kind == TY_PTR)
      error_tok(node->tok, "無効なポインタ演算オペランドです");
    node->type = node->lhs->type;
    return;
  case ND_SUB:
    if (node->rhs->type->kind == TY_PTR)
      error_tok(node->tok, "無効なポインタ演算オペランドです");
    node->type = node->lhs->type;
    return;
  case ND_ASSIGN:
    node->type = node->lhs->type;
    return;
  case ND_ADDR:
    node->type = pointer_to(node->lhs->type);
    return;
  case ND_DEREF:
    if (node->lhs->type->kind != TY_PTR)
      error_tok(node->tok, "無効なポインタ参照です");
    node->type = node->lhs->type->base;
    return;
  case ND_SIZEOF:
    node->kind = ND_NUM;
    node->type = int_type();
    node->val = size_of(node->lhs->type);
    node->lhs = NULL;
    return;
  }
}

void add_type(Function *prog) {
  for (Function *fn = prog; fn; fn = fn->next)
    for (Node *node = fn->node; node; node = node->next)
      visit(node);
}
