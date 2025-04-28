#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
  TK_RETURN,   // return を表すトークン
  TK_IF,       // if を表すトークン
  TK_ELSE,     // else を表すトークン
  TK_WHILE,    // while を表すトークン
  TK_FOR,      // for を表すトークン
  TK_BLOCK,    // {} を表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

typedef struct Var Var;
// 変数の型
struct Var {
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RPBからのオフセット
};

typedef struct VarList VarList;

// 変数のリスト
struct VarList {
  VarList *next; // 次の変数がNULL
  Var *var;
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident();
bool consume_kind();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();
char *strndup(char *p, int len);

// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

//
// parse.c
//

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_ASSIGN, // =
  ND_LVAR,   // ローカル変数
  ND_NUM,    // 整数
  ND_RETURN, // return
  ND_IF,     // if
  ND_WHILE,  // while
  ND_FOR,    // for
  ND_BLOCK,  // block {}
  ND_FUNCALL // Function call
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
  NodeKind kind; // ノードの型
  Node *next;    // 次のノード
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがNU_NUMの場合のみ使う
  Var *var;      // kindがND_LVARの場合のみ使う

  // "if", while, for  statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;
  // block {}
  Node *block;

  // function name
  char *funcname;
  // function args
  Node *args;
  int arg_count;
};

typedef struct Function Function;

struct Function {
  Function *next;

  // function name
  char *name;
  // params
  VarList *params;

  // body
  Node *node;

  // ローカル変数
  VarList *locals;

  // スタックサイズ
  int stack_size;
};

Function *program();

//
// codegen.c
//
void gen(Node *node);
void codegen(Function *prog);
void print_ast(Node *node, int depth);
