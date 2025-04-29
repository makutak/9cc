#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize();
  Function *prog = program();
  for (Function *fn = prog; fn; fn = fn->next) {
    int offset = 0;
    for (VarList *lvar = fn->locals; lvar; lvar = lvar->next) {
      offset += 8;
      lvar->var->offset = offset;
      /* printf("locals->var->offset: %d %p\n", lvar->var->offset, &lvar->var->offset); */
      /* printf("locals->var->name: %s\n", lvar->var->name); */
      /* printf("\n"); */
    }
    fn->stack_size = offset;

    /* for (VarList *lv = fn->params; lv; lv = lv->next) { */
    /*   printf("params->var->offset: %d %p \n", lv->var->offset, &lv->var->offset); */
    /*   printf("params->var->name: %s %p \n", lv->var->name, &lv->var->offset); */
    /*   printf("\n"); */
    /* } */

    /* print_ast(fn->node, 0); */
  }

  codegen(prog);

  return 0;
}
