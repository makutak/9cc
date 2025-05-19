.intel_syntax noprefix
.global main
main:
  mov rax, 1
  sete al
  movzx rax, al
  ret

