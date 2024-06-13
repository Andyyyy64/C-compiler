#include "9cc.h"

void gen_lval(Node *node) {
    if(node->kind != ND_LVAR) {
        error("Left value is not a variable");
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen(Node *node) {
    // generate assembly according to node type
    switch(node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf(" pop rdi\n");
    printf(" pop rax\n");

    switch(node->kind) {
        case '+':
            printf("  add rax, rdi\n");
            break;
        case '-':
            printf("  sub rax, rdi\n");
            break;
        case '*':
            printf("  imul rax, rdi\n");
            break;
        case '/':
            printf("  cqo\n");
            printf("  idiv rdi\n");
    }

    printf("  push rax\n");
}


void codegen(Node *node) {
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    // A result must be at the top of the stack, so pop it
    // to RAX to make it a program exit code(echo $?).
    printf(" pop rax\n");
    printf(" ret\n");
}
