# 1 "main.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "main.S"
# 1 "dl.h" 1
# 2 "main.S" 2

__hdr: 
    .ascii "\x01\x14\x05\x14"; 
    .4byte (__end - __hdr); 
    .4byte (__code - __hdr)

.align 32, 0; .8byte (0); .ascii "+" "libc.dl" "\0"
.align 32, 0; .8byte (0); .ascii "+" "libhello.dl" "\0"
.align 32, 0; 

hello: 
    .8byte (0); # actual address (to be filled on program runs)
    .ascii "?hello\0"

.align 32, 0; .8byte (main - __hdr); .ascii "#" "main" "\0"

.fill 32 - 1, 1, 0; .align 32, 0; __code:

main:
  call *hello(%rip)
  call *hello(%rip)
  call *hello(%rip)
  call *hello(%rip)
  movq $0, %rax
  ret

__end:


