#include "9cc.h"

VarList *locals;

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

// ローカル変数を名前で検索する。見つからなかった場合はNULLを返す
VarList *find_lvar(Token *tok) {
  for (VarList *lvar = locals; lvar; lvar = lvar->next)
    if (lvar->var->len == tok->len && !memcmp(tok->str, lvar->var->name, lvar->var->len))
      return lvar;
  return NULL;
}

Function *program();
Function *function();
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
  locals = NULL;

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

// function = ident "(" ")" "{" stmt* "}"
Function *function() {
  Function *fn = calloc(1, sizeof(Function));

  Token *tok = consume_ident();
  fn->name = strndup(tok->str, tok->len);

  expect("(");
  expect(")");
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

// stmt = exprt ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" "expr? "; expr? ")" stmt
//      | "return" expr ";"
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

  if (consume_kind(TK_IF)) {
    expect("(");
    Node *cond = expr();
    expect(")");

    Node *then = stmt();

    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->cond = cond;
    node->then = then;

    if (consume_kind(TK_ELSE)) {
      node->els = stmt();
    }

    return node;
  }

  if (consume_kind(TK_WHILE)) {
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

  if (consume_kind(TK_FOR)) {
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

  if (consume_kind(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

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

// unary = ("+" | "-")? primary
Node *unary() {
  for (;;) {
    if (consume("+"))
      return unary();
    if (consume("-"))
      return new_node(ND_SUB, new_node_num(0), unary());
    return primary();
  }
}

// func_args = "(" (expr ("," expr)*)? ")"
Node *func_args(Node *funcall) {
  funcall->arg_count = 0;
  if (consume(")"))
    return NULL;

  Node head = {};
  head.next = NULL;
  Node *cur = &head;

  while (!consume(")")) {
    cur->next = expr();
    cur = cur->next;
    funcall->arg_count++;

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
    Node *node = calloc(1, sizeof(Node));

    if (consume("(")) {
      node->kind = ND_FUNCALL;
      node->funcname = strndup(tok->str, tok->len);
      node->args = func_args(node);
      return node;
    }

    node->kind = ND_LVAR;
    VarList *lvar = find_lvar(tok);
    if (lvar) {
      node->var = lvar->var;
    } else {
      lvar = calloc(1, sizeof(VarList));
      lvar->next = locals;

      Var *var = calloc(1, sizeof(Var));
      var->name = tok->str;
      var->len = tok->len;
      // set lvar's var
      lvar->var = var;
      // set node var's var
      node->var = var;
      locals = lvar;
    }
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}
