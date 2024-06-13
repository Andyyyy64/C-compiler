#include "9cc.h"

char *user_input;
Token *token;

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
  if(token->kind != TK_RESEREVED || strlen(op) != token->len ||
     memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next; // move to next token
  return true;
}

char *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }
  char *name = calloc(token->len + 1, sizeof(char));
  strncpy(name, token->str, token->len);
  token = token->next;
  return name;
}

// if the next token is the expected symbol, read the token otherwise report an error.
void expect(char *op) {
  if(token->kind != TK_RESEREVED || strlen(op) != token->len ||
  memcmp(token->str, op, token->len)) {
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  // initialize token
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  // connect to cur
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p,q,strlen(q)) == 0;
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

    // multi-letter punctuator
    if(startswith(p, "==") || startswith(p, "!=") ||
       startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESEREVED , cur, p, 2);
      p += 2;
      continue;
    }

    // single-letter punctuator
    if(strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESEREVED, cur, p++, 1);
      continue;
    }

    // integer literal
    if(isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    if('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 1);
      cur->len = 1;
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
