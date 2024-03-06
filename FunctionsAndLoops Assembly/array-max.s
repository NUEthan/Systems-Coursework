# Write the assembly code for the array_max function
# Stack method didn't rly work :/ or atleast I couldn't get it to work...
.global array_max

.text

loop:
  cmp %r12, %r14     #arraylength  and cur index

  jge end_loop

  mov (%r13, %r14, 8), %rdx

  cmp %rax, %rdx

  jge increment_index

  inc %r14	      #i++
  jmp loop


array_max:
  enter $8, $0       #keep, for space allocate

  mov %rdi, %r12     #fir arg
  mov %rsi, %r13     #sec arg

  mov $0, %rax        #max=0
  mov $0, %r14        #i=0

  jmp loop

  leave
  ret


end_loop:
        leave    #clean :3
        ret


increment_index:
        mov %rdx, %rax    # update to new max
        inc %r14          #i++
        jmp loop


.data
format: .asciz "%ld\n"
