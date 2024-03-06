# Write the assembly code for the main of the compare program
.global main

.text

main:
  enter $0, $0

  movq 8(%rsi), %r12      #Load Args
  movq 16(%rsi), %r13
  movq 32(%rsi), %r14

  cmp $0, %r12             #check if Args is 0

  je is_zero

  cmp $0, %r13

  je is_zero


  mov %r12, %rdi         # convert first arg 

  call atol

  mov %rax, %r12

  mov %r13, %rdi         # convert second arg

  call atol

  mov %rax, %r13

  cmp $0, %r14           #check third arg = 0?
  je is_zero

  cmp %r13, %r12         #compare then send to whatever

  je equal
  jg greater
  jl less

  jmp is_zero


equal:
  mov $equalSTR, %rdi
  mov $0, %al

  call printf

  jmp done

greater:
  mov $greaterSTR, %rdi
  mov $0, %al

  call printf

  jmp done

less:
  mov $lessSTR, %rdi
  mov $0, %al

  call printf

  jmp done

is_zero:
  mov $errorSTR, %rdi
  mov $0, %al

  call printf

  jmp done

done:
  leave
  ret

data:
  errorSTR: .string "Invalid Input.\n"
  equalSTR: .string "equal\n"
  greaterSTR: .string "greater\n"
  lessSTR: .string "less\n"

format:
  .asciz "%d\n"

# ./compare arg1 arg2
