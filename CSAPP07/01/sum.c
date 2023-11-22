int sum(int *array, int n) { // 定义符号
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += array[i];
    }
    return total;
}
