#include "9cc.h"

char *read_file(char *path) {
  // ファイルを開く
  FILE *fp = fopen(path, "r");
  if (!fp)
    error("cannot open %s: %s", path, strerror(errno));

  // ファイルの長さを調べる
  if (fseek(fp, 0, SEEK_END) == -1)
    error("%s; fseek: %s", path, strerror(errno));

  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  // ファイル内容を読み込む
  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  // ファイルが必ず"\n\0"で終っているようにする
  if (size == 0 || buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return buf;
}

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: 引数の個数が正しくありません", argv[0]);

  // トークナイズしてパースする
  // user_input = argv[1];
  filename = argv[1];
  user_input = read_file(filename);
  token = tokenize();
  Program *prog = program();
  add_type(prog);

  for (Function *fn = prog->fns; fn; fn = fn->next) {
    int offset = 0;
    for (VarList *lvar = fn->locals; lvar; lvar = lvar->next) {
      Var *var = lvar->var;
      offset += size_of(var->type);
      /* printf("locals->var->offset: %d %p\n", lvar->var->offset, &lvar->var->offset); */
      /* printf("locals->var->name: %s\n", lvar->var->name); */
      /* printf("\n"); */
      var->offset = offset;
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
