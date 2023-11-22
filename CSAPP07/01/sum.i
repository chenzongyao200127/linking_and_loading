# 1 "sum.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "sum.c"
int sum(int *array, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += array[i];
    }
    return total;
}
