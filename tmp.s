.intel_syntax noprefix
.globl main
main:
  push 12
  push 10
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  pop rax
  ret
