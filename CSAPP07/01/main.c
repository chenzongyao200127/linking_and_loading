#include <stdio.h>

extern int sum(int *, int); // 引用符号

int main() {
    int array[2] = {1, 2};
    int result = sum(array, 2); // 对 sum 函数的引用
    printf("Result is %d\n", result);
    return 0;
}
