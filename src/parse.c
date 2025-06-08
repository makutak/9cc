#include "9cc.h"

VarList *locals;
VarList *globals;
VarList *scope;

Var *push_var(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;

  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
  return var;
}

Var *create_var(char *name, Type *type, bool is_local) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->type = type;
  var->is_local = is_local;

  VarList *sc = calloc(1, sizeof(VarList));
  sc->var = var;
  sc->next = scope;
  scope = sc;

  return var;
}

void register_local(Var *var) {
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
}

void register_globals(Var *var) {
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = globals;
  globals = vl;
}

Node *new_node(NodeKind kind, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  return node;
}

Node *new_binary_node(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_unary_node(NodeKind kind, Node *lhs, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  return node;
}

Node *new_num_node(int val, Token *tok) {
  Node *node = new_node(ND_NUM, tok);
  node->val = val;
  return node;
}

Node *new_var_node(Var *var, Token *tok) {
  Node *node = new_node(ND_VAR, tok);
  node->var = var;
  return node;
}

Node *new_funcall_node(char *funcname, Node *args, Token *tok) {
  Node *node = new_node(ND_FUNCALL, tok);
  node->funcname = funcname;
  node->args = args;
  return node;
}

// ローカル変数かグローバル変数を名前で検索する。見つからなかった場合はNULLを返す
Var *find_var(Token *tok) {
  for (VarList *vl = scope; vl; vl = vl->next) {
    Var *var = vl->var;
    if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len))
      return var;
  }

  return NULL;
}

char *new_label() {
  static int cnt = 0;
  char buf[20];
  sprintf(buf, ".L.data.%d", cnt++);
  return strndup(buf, 20);
}

Program *program();
Function *function();
Type *basetype();
void global_var();
Node *declaration();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *postfix();
Node *primary();

bool is_function() {
  Token *tok = token;
  basetype();
  bool isfunc = consume_ident() && consume("(");
  token = tok;
  return isfunc;
}

VarList *read_params() {
  if (consume(")"))
    return NULL;

  VarList head = {};
  head.next = NULL;
  VarList *cur = &head;

  while (!consume(")")) {
    Type *type = basetype();

    cur->next = calloc(1, sizeof(VarList));
    Var *var = create_var(expect_ident(), type, true);
    cur->next->var = var;
    register_local(var);
    cur = cur->next;
    if (!consume(",")) {
      break;
    }
  }

  expect(")");
  cur->next = NULL;

  return head.next;
}

Type *read_type_suffix(Type *base) {
  if (!consume("["))
    return base;

  int size = expect_number();
  expect("]");
  base = read_type_suffix(base);
  return array_type(base, size);
}

bool is_typename() {
  return peek("int") || peek("char");
}

// program = (global-var | function)*
Program *program() {
  Function head = {};
  head.next = NULL;
  Function *cur = &head;
  globals = NULL;

  while (!at_eof()) {
    if (is_function()) {
      cur->next = function();
      cur = cur->next;
    } else {
      global_var();
    }
  }
  cur->next = NULL;

  Program *prog = calloc(1, sizeof(Program));
  prog->fns = head.next;
  prog->globals = globals;
  return prog;
}

// basetype = ("char" | "int" | "void") "*"*
Type *basetype() {
  Type *type;

  if (consume("char")) {
    type = char_type();
  } else if (consume("int")) {
    type = int_type();
  } else {
    expect("void");
    type = int_type();
  }

  while (consume("*"))
    type = pointer_to(type);

  return type;
}

// global-var = basetype ident ("[" num "]")* ";"
void global_var() {
  Type *type = basetype();
  char *name = expect_ident();
  type = read_type_suffix(type);
  expect(";");
  Var *var = create_var(name, type, false);
  register_globals(var);
}

// function = ident "(" params? ")" "{" stmt* "}"
// params = basetype ident ("," basetype ident)*
Function *function() {
  locals = NULL;

  Function *fn = calloc(1, sizeof(Function));
  basetype();
  fn->name = expect_ident();
  expect("(");
  fn->params = read_params();
  expect("{");

  Node head = {};
  head.next = NULL;
  Node *cur = &head;

  while (!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }
  cur->next = NULL;

  fn->node = head.next;
  fn->locals = locals;
  fn->next = NULL;

  return fn;
}

// declaration = basetype ident ("[" num "]")* ("=" expr) ";"
Node *declaration() {
  Token *tok = token;

  // 新しい変数宣言
  Type *type = basetype();
  char *name = expect_ident();

  type = read_type_suffix(type);
  Var *var = create_var(name, type, true);
  register_local(var);

  if (consume(";"))
    return new_node(ND_NULL, tok);

  expect("=");
  Node *lhs = new_var_node(var, tok);
  Node *rhs = expr();
  expect(";");
  Node *node = new_binary_node(ND_ASSIGN, lhs, rhs, tok);
  return new_unary_node(ND_EXPR_STMT, node, tok);
}

Node *expr_stmt() {
  Token *tok = token;
  return new_unary_node(ND_EXPR_STMT, expr(), tok);
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" "expr? "; expr? ")" stmt
//      | "{" stmt* "}"
//      | declaration
//      | expr ";"
Node *stmt() {
  Token *tok;

  if (tok = consume("return")) {
    Node *node = new_unary_node(ND_RETURN, expr(), tok);
    expect(";");
    return node;
  }

  if (tok = consume("if")) {
    Node *node = new_node(ND_IF, tok);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();

    if (consume("else")) {
      node->els = stmt();
    }

    return node;
  }

  if (tok = consume("while")) {
    Node *node = new_node(ND_WHILE, tok);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();

    return node;
  }

  if (tok = consume("for")) {
    Node *node = new_node(ND_FOR, tok);
    expect("(");
    if (!consume(";")) {
      node->init = expr_stmt();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr_stmt();
      expect(")");
    }
    node->then = stmt();

    return node;
  }

  if (tok = consume("{")) {
    Node block_head = {};
    block_head.next = NULL;
    Node *cur = &block_head;

    VarList *sc = scope;
    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    cur->next = NULL;
    scope = sc;

    Node *node = new_node(ND_BLOCK, tok);
    node->block = block_head.next;

    return node;
  }

  if (is_typename())
    return declaration();

  Node *node = expr_stmt();

  if (!consume(";"))
    error_at(token->str, "';'ではないトークンです");

  return node;
}

// exprt = assign
Node *expr() {
  return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();

  Token *tok;
  if (tok = consume("="))
    node = new_binary_node(ND_ASSIGN, node, assign(), tok);
  return node;
}

// equality = relational ( "==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();
  Token *tok;

  for (;;) {
    if (tok = consume("=="))
      node = new_binary_node(ND_EQ, node, relational(), tok);
    else if (tok = consume("!="))
      node = new_binary_node(ND_NE, node, relational(), tok);
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add  | ">=" add)*
Node *relational() {
  Node *node = add();
  Token *tok;

  for (;;) {
    if (tok = consume("<"))
      node = new_binary_node(ND_LT, node, add(), tok);
    else if (tok = consume("<="))
      node = new_binary_node(ND_LE, node, add(), tok);
    else if (tok = consume(">"))
      node = new_binary_node(ND_LT, add(), node, tok);
    else if (tok = consume(">="))
      node = new_binary_node(ND_LE, add(), node, tok);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();
  Token *tok;

  for (;;) {
    if (tok = consume("+"))
      node = new_binary_node(ND_ADD, node, mul(), tok);
    else if (tok = consume("-"))
      node = new_binary_node(ND_SUB, node, mul(), tok);
    else
      return node;
  }
}

// mul  = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();
  Token *tok;

  for (;;) {
    if (tok = consume("*"))
      node = new_binary_node(ND_MUL, node, unary(), token);
    else if (tok = consume("/"))
      node = new_binary_node(ND_DIV, node, unary(), token);
    else
      return node;
  }
}

// unary = ("+" | "-" | "*" | "&")? unary
//       | "sizeof" unary
//       | postfix
Node *unary() {
  Token *tok;

  if (consume("+"))
    return unary();

  if (tok = consume("-"))
    return new_binary_node(ND_SUB, new_num_node(0, tok), unary(), tok);

  if (tok = consume("&"))
    return new_unary_node(ND_ADDR, unary(), tok);

  if (tok = consume("*"))
    return new_unary_node(ND_DEREF, unary(), tok);

  if (tok = consume("sizeof"))
    return new_unary_node(ND_SIZEOF, unary(), tok);

  return postfix();
}

// postfix = primary ("[" expr "]")*
Node *postfix() {
  Node *node = primary();
  Token *tok;

  while (tok = consume("[")) {
    Node *exp = new_binary_node(ND_ADD, node, expr(), tok);
    expect("]");
    node = new_unary_node(ND_DEREF, exp, tok);
  }

  return node;
}

// stmt-expr = "(" "{" stmt stmt* "}" ")"
//
// Statement expression is a GNU C extension.
Node *stmt_expr(Token *tok) {
  VarList *sc = scope;

  Node *node = new_node(ND_STMT_EXPR, tok);
  node->block = stmt();
  Node *cur = node->block;

  while (!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }
  expect(")");

  scope = sc;

  if (cur->kind != ND_EXPR_STMT)
    error_tok(cur->tok, "stmt expr returning void is not supported");
  *cur = *cur->lhs;
  return node;
}

// func_args = "(" (expr ("," expr)*)? ")"
Node *func_args() {
  if (consume(")"))
    return NULL;

  Node head = {};
  head.next = NULL;
  Node *cur = &head;

  while (!consume(")")) {
    cur->next = assign();
    cur = cur->next;

    if (!consume(","))
      break;
  }

  expect(")");
  cur->next = NULL;

  return head.next;
}

// primary = "(" expr ")"
//         | "(" "{" stmt-expr-tail
//         | ident func_args?
//         | str
//         | num
Node *primary() {
  Token *tok;

  if (consume("(")) {
    if (consume("{"))
      return stmt_expr(token);

    Node *node = expr();
    expect(")");
    return node;
  }

  if (tok = consume_ident()) {
    // 関数呼び出し
    if (consume("(")) {
      return new_funcall_node(strndup(tok->str, tok->len), func_args(), tok);
    }

    // 変数使用
    Var *var = find_var(tok);
    if (!var)
      error_at(tok->str, "宣言されていない変数です");
    return new_var_node(var, tok);
  }

  tok = token;
  if (tok->kind == TK_STR) {
    token = token->next;

    Type *type = array_type(char_type(), tok->cont_len);
    Var *var = create_var(new_label(), type, false);
    register_globals(var);
    var->contents = tok->contents;
    var->cont_len = tok->cont_len;
    return new_var_node(var, tok);
  }

  if (tok->kind != TK_NUM)
    error_tok(tok, "数値ではありません");
  // それ以外は数値リテラル
  return new_num_node(expect_number(), token);
}
