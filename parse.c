#include "9cc.h"

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

void program();     // program    = stmt*
Node *stmt();       // stmt       = expr ";"
Node *expr();       // expr       = assign
Node *assign();     // assign     = equality ("=" assign)?
Node *equality();   // equality   = relational ("==" relational | "!=" relational)*
Node *relational(); // relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *add();        // add        = mul ("+" mul | "-" mul)*
Node *mul();        // mul        = unary ("*" unary | "/" unary)*
Node *unary();      // unary      = ("+" | "-")? primary
Node *primary();    // primary    = num | "(" expr ")"

Node *code[100];

void program() {
  int i = 0;
  while(!at_eof()) { // until end of input
    code[i++] = stmt(); // parse statement
  }
  code[i] = NULL; // set NULL to the end of code
}

// expr = equality
Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();

  if(consume("=")) {
    node = new_node(ND_ASSIGN, node, assign()); // create new node for assignment
  }
}

Node *stmt() {
  Node *node = expr();
  expect(";");
  return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for(;;) {
    if(consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if(consume("!=")) {
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for(;;) {
    if(consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if(consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if(consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else if(consume(">=")) {
      node = new_node(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for(;;) {
    if(consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if(consume("-")) {
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
    if(consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if(consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// unary = ("+" | "-")? primary
Node *unary() {
  if(consume("+")) {
    return primary(); // +x to x
  }
  if(consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());  // -x to 0-x
  }
  return primary();
}

// primary = num | "(" expr ")"
Node *primary() {
  // if next token is '(', it should be '(' expr ')'
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  Token *tok = consume_ident();
  if(tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }
  return new_node_num(expect_number());
}
