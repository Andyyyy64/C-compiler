#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// token type
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
};

Token *token; // current token

// error report function
// takes the same arguments as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// if the next token is the expected symbol, read the token return true.
bool consume(char op) {
  if(token->kind != TK_RESEREVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

// if the next token is the expected symbol, read the token otherwise report an error.
void expect(char op) {
  if(token->kind != TK_RESEREVED || token->str[0] != op) {
    error("''%c'ではありません", op);
    token = token->next;
  }
}

// if the next token is a number, read the token and return the number.
int expect_number() {
  if(token->kind != TK_NUM) {
    error("数ではありません");
  }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p) {
    // skip whitespace characters
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(*p == '+' || *p == '-') {
      cur = new_token(TK_RESEREVED, cur, p++);
      continue;
    }

    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // tokenize and parse
  token = tokenize(argv[1]);

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // the first token must be a number
  printf("  mov rax, %d\n", expect_number());

  while(!at_eof()) {
    if(consume('+')) {
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}
