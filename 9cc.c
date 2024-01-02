#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//
// Tokenizer
//

// token type enum
typedef enum {
  TK_RESEREVED, // reserved word
  TK_NUM,       // integer token
  TK_EOF,       // end of file token
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind; // token type
  Token *next; // next input token
  int val; // if kind is TK_NUM, this is the number
  char *str; // token string
  int len; // token length
};


Token *token; // current token
char *user_input; // input program

// error report function
// takes the same arguments as printf
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input; // get error position
  fprintf(stderr, "%s\n", user_input); // print input program
  fprintf(stderr, "%*s", pos, ""); // print pos spaces
  fprintf(stderr, "^ "); // print error mark
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// if the next token is the expected symbol, read the token return true.
bool consume(char *op) {
  if(token->kind != TK_RESEREVED ||
     strlen(op) != token->len ||
     memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next; // move to next token
  return true;
}

// if the next token is the expected symbol, read the token otherwise report an error.
void expect(char op) {
  if(token->kind != TK_RESEREVED || token->str[0] != op) {
    error_at(token->str, "expected '%c'", op);
  }
  token = token->next; // move to next token
}

// if the next token is a number, read the token and return the number.
int expect_number() {
  if(token->kind != TK_NUM) {
    error_at(token->str, "expected a number");
  }
    int val = token->val;
    token = token->next; // move to next token
    return val;
}

// detect end of input
bool at_eof() {
  return token->kind == TK_EOF;
}

// create new token and connect it to cur
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  // initialize token
  tok->kind = kind;
  tok->str = str;
  // connect to cur
  cur->next = tok;
  return tok;
}

// parse input and return new token
Token *tokenize() {
  char *p = user_input; // get input program from global variable
  Token head; // dummy head(banpei)
  head.next = NULL; // initialize next token
  Token *cur = &head; // current token

  while(*p) {
    // skip whitespace characters
    if(isspace(*p)) {
      p++;
      continue;
    }

    // reserved word
    if(strchr("+-*/()", *p)) {
      cur = new_token(TK_RESEREVED, cur, p++);
      continue;
    }

    // integer literal
    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

//
// Parser
//

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // integer
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind; // node type
  Node *lhs; // left-hand side(左辺)
  Node *rhs; // right-hand side(右辺)
  int val; // if kind is ND_NUM, this is the number
};

// create new node and connect it to lhs and rhs
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// create new node for integer
Node *new_node_num(int val) {
  Node *node = calloc(1,sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *expr();       // expr       = equality
Node *equality();   // equality   = relational ("==" relational | "!=" relational)*
Node *relational(); // relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *add();        // add        = mul ("+" mul | "-" mul)*
Node *mul();        // mul        = unary ("*" unary | "/" unary)*
Node *unary();      // unary      = ("+" | "-")? primary
Node *primary();    // primary    = num | "(" expr ")"

// expr = mul ("+" mul | "-" mul)*
Node *expr() {
  Node *node = mul();

  for(;;) {
    if(consume('+')) {
      node = new_node(ND_ADD, node, mul());
    }
      else if(consume('-')) {
        node = new_node(ND_SUB, node, mul());
      } else {
        return node;
      }
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for(;;) {
    if(consume('*')) {
      node = new_node(ND_MUL, node, unary());
    } else if(consume('/')) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// unary = ("+" | "-")? primary
Node *unary() {
  if(consume('+')) {
    return primary(); // +x to x
  }
  if(consume('-')) {
    return new_node(ND_SUB, new_node_num(0), primary());  // -x to 0-x
  }
  return primary();
}

// primary = num | "(" expr ")"
Node *primary() {
  // if next token is '(', it should be '(' expr ')'
  if(consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  return new_node_num(expect_number());
}

void gen(Node *node) {
  if(node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  // generate assembly according to node type
  switch(node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
  }

  printf("  push rax\n");
}


int main(int argc, char **argv) {
  if (argc != 2) {
    error_at("%s: invalid number of arguments\n", argv[0]);
  }

  // store input program
  user_input = argv[1];

  // tokenize and parse
  token = tokenize();
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // going down the AST to generate assembly
  gen(node);

  // there must be a single value on the stack after the expression
  // so pop it to RAX to make it a program exit code
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
