#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//
// tokenize.c
//

// token type enum
typedef enum {
  TK_RESEREVED, // reserved word
  TK_IDENT,     // identifier token
  TK_NUM,       // integer token
  TK_EOF,       // end of file token
} TokenKind;

// token type
typedef struct Token Token;
struct Token {
  TokenKind kind; // token type
  Token *next; // next input token
  int val; // if kind is TK_NUM, this is the number
  char *str; // token string
  int len; // token length
};

void error(char *fmt, ...); // print error
void error_at(char *loc, char *fmt, ...); // print whare is the error at
bool consume(char *op);
char *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();

extern char *user_input;
extern Token *token;


//
// parse.c
//

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_LVAR, // local variable
  ND_ASSIGN, // =
  ND_NUM, // integer
  ND_EQ, // ==
  ND_NE, // !=
  ND_LT, // <
  ND_LE, // <=
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind; // node type
  Node *lhs; // left-hand side(左辺)
  Node *rhs; // right-hand side(右辺)
  int val; // if kind is ND_NUM, this is the number
  int offset; // if kind is ND_LVAR, this is the offset from the base pointer
};

void program();
Node *expr();

Node *code[100];

//
// codegen.c
//

void gen(Node *node);
void codegen(Node *node);
