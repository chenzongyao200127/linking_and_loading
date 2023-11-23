#include <stdio.h>

static int staticVar = 10; // 局部符号

void globalFunction() { // 全局符号
    static int insideStaticVar = 5; // 局部符号，但在函数内部
    printf("This is a global function.\n");
}

int main() {
    globalFunction(); // 引用全局符号
    return 0;
}
