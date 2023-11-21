#include <sys/syscall.h>  // Include for system call numbers

.globl _start            // Global label for the entry point
_start:                  // Entry point of the program

  movq $SYS_write, %rax  // write(
  movq $1, %rdi          //   fd=1, (file descriptor 1 is standard output)
  lea  st(%rip), %rsi    //   buf=st, (load the address of the string into RSI)
  movq $(ed - st), %rdx  //   count=ed-st (calculate the length of the string)
  syscall                // ); (perform the system call)

  movq $SYS_exit, %rax   // exit(
  movq $1, %rdi          //   status=1 (exit status 1)
  syscall                // ); (perform the system call)

st:                      // Start label for the string
  .ascii "\033[01;31mHello, OS World\033[0m\n" // The string to print with ANSI color codes
ed:                      // End label for the string
