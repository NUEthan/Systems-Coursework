#
# Usage: ./calculator <op> <arg1> <arg2>
#

# Make `main` accessible outside of this module
.global main

# Start of the code section
.text

# int main(int argc, char argv[][])
main:
  # Function prologue
  enter $0, $0

  # Variable mappings:
  # op -> %r12
  # arg1 -> %r13
  # arg2 -> %r14
  movq 8(%rsi), %r12  # op = argv[1]
  movq 16(%rsi), %r13 # arg1 = argv[2]
  movq 24(%rsi), %r14 # arg2 = argv[3]


  # Hint: Convert 1st operand to long int
  mov %r13, %rdi
  call atol         # note: atol converts to long int
  mov %rax, %r13

  # Hint: Convert 2nd operand to long int
  mov %r14, %rdi
  call atol
  mov %rax, %r14

  # Hint: Copy the first char of op into an 8-bit register

  mov %r12, %rdi

  mov $addX, %rsi
  call strcmp
  cmp $0, %rax
  je add

  mov $subX, %rsi
  call strcmp
  cmp $0, %rax
  je sub

  mov $divX, %rsi
  call strcmp
  cmp $0, %rax
  je div

  mov $mulX, %rsi
  call strcmp
  cmp $0, %rax
  je mul

  jne input_error

add:
  add %r13, %rax
  add %r14, %rax
  jmp print_final

sub:
  mov %r13, %rax
  sub %r14, %rax
  jmp print_final

div:
  mov $0, %rdx
  mov %r13, %rax
  cqto             #
  idiv %r14
  jmp print_final

mul:
  mov $1, %rax
  imul %r13
  imul %r14
  jmp print_final

print_final:
  mov $resultMessage, %rdi
  mov %rax, %rsi
  call printf
  jmp finish_program


input_error:
  mov $errorMessage, %rdi
  mov %r12, %rsi
  call printf

finish_program:
  leave
  ret


  # Function epilogue
  leave
  ret


# Start of the data section
.data
  addX: .string "+"
  subX: .string "-"
  divX: .string "/"
  mulX: .string "*"

  resultMessage: .asciz "%ld\n"
  errorMessage: .asciz "Unknown operation\n"

format: 
  .asciz "%ld\n"

