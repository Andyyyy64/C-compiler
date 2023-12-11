.intel_syntax noprefix
    .global plus, main
plus:
    add rsi, rdi
    mov rax, rsi
    ret
main:
    mov rdi, 20
    mov rsi, 21
    call plus
    ret
