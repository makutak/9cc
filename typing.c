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
