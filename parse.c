#include "9cc.h"

VarList *locals;

Var *push_var(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;

  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
  return var;
}

Var *create_var(char *name, Type *ty) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->ty = ty;
  return var;
}

void register_local(Var *var) {
  VarList *vl = calloc(1, sizeof(VarList));
  vl->var = var;
  vl->next = locals;
  locals = vl;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *new_unary(NodeKind kind, Node *lhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  return node;
}

Node *new_var_node(Var *var) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_VAR;
  node->var = var;
  return node;
}

Node *new_funcall_node(char *funcname, Node *args) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNCALL;
  node->funcname = funcname;
  node->args = args;
  return node;
}

// ローカル変数を名前で検索する。見つからなかった場合はNULLを返す
Var *find_var(Token *tok) {
  for (VarList *vl = locals; vl; vl = vl->next) {
    Var *var = vl->var;
    if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len))
      return var;
  }

  return NULL;
}

Function *program();
Function *function();
VarList *read_params();
Node *declaration();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// program = function*
Function *program() {
  Function head = {};
  head.next = NULL;
  Function *cur = &head;
  while (!at_eof()) {
    cur->next = function();
    cur = cur->next;
  }
  cur->next = NULL;

  return head.next;
}

// basetype = "int" "*"*
Type *basetype() {
  expect("int");
  Type *type = int_type();
  while (consume("*"))
    type = pointer_to(type);

  return type;
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

VarList *read_params() {
  if (consume(")"))
    return NULL;

  VarList head = {};
  head.next = NULL;
  VarList *cur = &head;

  while (!consume(")")) {
    Type *type = basetype();

    cur->next = calloc(1, sizeof(VarList));
    Var *var = create_var(expect_ident(), type);
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

// declaration = basetype ident ("=" expr) ";"
Node *declaration() {
  // 新しい変数宣言
  Type *type = basetype();
  Token *tok = consume_ident();
  if (!tok)
    error_at(token->str, "変数名が必要です");

  Var *var = create_var(strndup(tok->str, tok->len), type);
  register_local(var);
  if (consume(";"))
    return new_var_node(var);

  expect("=");
  Node *lhs = new_var_node(var);
  Node *rhs = expr();
  expect(";");
  return new_node(ND_ASSIGN, lhs, rhs);
}

// stmt = expr ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" "expr? "; expr? ")" stmt
//      | "return" expr ";"
//      | declaration
Node *stmt() {
  Node *node;

  if (consume("{")) {
    Node block_head = {};
    block_head.next = NULL;
    Node *cur = &block_head;
    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    cur->next = NULL;

    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->block = block_head.next;

    return node;
  }

  if (consume("if")) {
    expect("(");
    Node *cond = expr();
    expect(")");

    Node *then = stmt();

    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->cond = cond;
    node->then = then;

    if (consume("else")) {
      node->els = stmt();
    }

    return node;
  }

  if (consume("while")) {
    expect("(");
    Node *cond = expr();
    expect(")");

    Node *then = stmt();
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->cond = cond;
    node->then = then;

    return node;
  }

  if (consume("for")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr();
      expect(")");
    }
    node->then = stmt();

    return node;
  }

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
    return node;
  }

  if (peek("int")) {
    return declaration();
  }

  node = expr();

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
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ( "==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add  | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul  = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = "+"? primary
//       | "-"? primary
//       | "*" unary
//       | "&" unary
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), unary());

  if (consume("&"))
    return new_unary(ND_ADDR, unary());

  if (consume("*"))
    return new_unary(ND_DEREF, unary());

  return primary();
}

// func_args = "(" (expr ("," expr)*)? ")"
Node *func_args() {
  if (consume(")"))
    return NULL;

  Node head = {};
  head.next = NULL;
  Node *cur = &head;

  while (!consume(")")) {
    cur->next = expr();
    cur = cur->next;

    if (!consume(","))
      break;
  }

  expect(")");
  cur->next = NULL;

  return head.next;
}

// primary = num
//         | ident func_args?
//         | "(" expr ")"
Node *primary() {
  // 次のトークンが"("なら、"(" expr ")" のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    // 関数呼び出し
    if (consume("(")) {
      return new_funcall_node(strndup(tok->str, tok->len), func_args());
    }
    // 変数使用
    Var *var = find_var(tok);
    if (!var)
      error_at(tok->str, "宣言されていない変数です");
    return new_var_node(var);
  }

  // それ以外は数値リテラル
  return new_node_num(expect_number());
}
