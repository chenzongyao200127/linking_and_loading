#include <sys/syscall.h>     // 包含系统调用号的定义

.globl _start              // 声明 _start 为全局入口点
_start:                    // 程序入口点
  movq $SYS_write, %rax    // write(，设置系统调用号为 write
  movq $1, %rdi            //   fd=1, 文件描述符 1 (标准输出)
  lea  st(%rip), %rsi      //   buf=st, 指向字符串的指针
  movq $(ed - st), %rdx    //   count=ed-st 字符串的长度
  syscall                  // ); 执行系统调用

  movq $SYS_exit, %rax     // exit(，设置系统调用号为 exit
  movq $1, %rdi            //   status=1 退出状态为 1
  syscall                  // ); 执行系统调用

st:                        // 字符串的起始标签
  .ascii "\033[01;31mThis is a loader.\033[0m\n" // ANSI 颜色代码和文本
ed:                        // 字符串的结束标签
