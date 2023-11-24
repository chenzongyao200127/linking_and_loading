# Loading and Linking Shared Libraries from Applications
# 从应用程序中加载和链接共享库

到目前为止，我们已经讨论了在应用程序被加载后执行前时，动态链接器加载和链接共享库的情景。
然而，应用程序还可能在它*运行时*要求动态链接器加载和链接某个共享库，而无需在编译时将那些库链接到应用中。


动态链接是一项强大有用的技术。下面是一些现实世界中的例子：
 - 分发软件。微软 Wmdows 应用的开发者常常利用共享库来分发软件更新。
 他们生成一个共库的新版本，然后用户可以下载，并用它替代当前的版本。
 下一次他们运行应用程序时，应用将自动链接和加载新的共享库。

 - 构建高性能 Web 服务器。许多 Web 服务器生成动态内容，比如个性化的 Web 页面、账户余额和广告标语 s 早期的 Web 服务器通过使用 fork 和 execve 创建一个子进程，并在该子进程的上下文中运行 CGI 程序来生成动态内容。
 然而，现代高性能的 Web 服务器可以使用基于动态链接的更有效和完善的方法来生成动态内容。

其思路是将每个生成动态内容的函数打包在共享库中。
当一个来自 Web 浏览器的请求到达时，服务器动态地加载和链接适当的函数，然后直接调用它，而不是使用 fork 和 execve 在子进程的上下文中运行函数。函数会一直缓存在服务器的地址空间中，所以只要一个简单的函数调用的开销就可以处理随后的请求了。
这对一个繁忙的网站来说是有很大影响的。**更进一步地说，在运行时无需停止服务器，就可以更新已存在的函数，以及添加新的函数**。
(软件动态更新技术)

Linux 系统为动态链接器提供了一个简单的接口，允许应用程序在运行时加载和链接共享库

~~~c
#include <dlfcn.h>

void *dlopen(const char *filename, int flag);

// 返回：若成功则为指向句柄的指针，若出错则为 NULL。
~~~

`dlopen` 函数加载和链接共享库 `filenameo` 用已用带 `RTLD_GLOBAL` 选项打开了的库解析 filename 中的外部符号。
如果当前可执行文件是带 `-rdynamic` 选项编译的，那么对符号解析而言，它的全局符号也是可用的。

flag 参数必须要么包括 `RTLD_NOW`，该标志告诉链接器立即解析对外部符号的引用，要么包括 `RTLD_LAZY` 标志，该标志指示链接器推迟符号解析直到执行来自库中的代码。这两个值中的任意一个都可以和 RTLD_GLOBAL 标志取或。

2. **`-rdynamic`**：这个选项用于告诉编译器将所有符号（不仅仅是程序使用的符号）添加到动态符号表中，
这对于使用动态加载机制（如 `dlopen`）的程序是必要的。它使得在运行时能够通过这些符号来解析函数或变量的地址。

~~~c
#include <dlfcn.h>

void *dlsym(void *handle, char *symbol);

// 返回：若成功则为指向符号的指针，若出错则为 NULL。
~~~

如果没有其他共享库还在使用这个共享库，dlclose函数就卸载该共享库。(引用计数)

~~~c
#include <dlfcn.h>

const char *dlerror(void);

// 返回：如果前面对 dlopen、dlsym 或 dlclose 的调用失败，
// 则为错误消息，如果前面的调用成功，则为 NULL。
~~~

`dlerror` 函数返回一个字符串，它描述的是调用 dlopen、dlsym 或者 dlclose 函数时发生的最近的错误，如果没有错误发生，就返回 NULL。

图 7-17 展示了如何利用这个接口动态链接我们的 `libvector.so` 共享库，然后调用它的 `addvec` 例程。
要编译这个程序，我们将以下面的方式调用 GCC：

~~~shell
linux> gcc -rdynamic -o prog2r dll.c -ldl
~~~

====================================================================================================
这个命令是在 Linux 环境下使用 GCC 编译器来编译一个程序，并链接动态链接库的一个例子。让我们逐部分地解析这个命令：

1. **`gcc`**：这是 GNU 编译器集合中的 C 编译器，用于编译 C 语言源代码。

2. **`-rdynamic`**：这个选项用于告诉编译器将所有符号（不仅仅是程序使用的符号）添加到动态符号表中，这对于使用动态加载机制（如 `dlopen`）的程序是必要的。它使得在运行时能够通过这些符号来解析函数或变量的地址。

3. **`-o prog2r`**：这个选项指定了输出的可执行文件名称，这里文件名是 `prog2r`。

4. **`dll.c`**：这是源代码文件，包含了程序的代码。假设 `dll.c` 中包含了动态加载库相关的代码。

5. **`-ldl`**：这个选项告诉编译器链接标准的动态链接库（libdl）。`libdl` 提供了加载和卸载共享库（动态链接库）的函数，如 `dlopen`、`dlclose`、`dlsym` 等。这些函数使得程序能够在运行时动态地加载和使用共享库。

综合以上信息，这个命令编译 `dll.c` 并创建一个名为 `prog2r` 的可执行文件，它链接了动态链接库，特别是那些支持动态加载共享对象（SO，即共享库）的功能。这类程序通常在运行时动态地加载和调用库中的函数，而不是在编译时静态地链接这些库。
====================================================================================================

示例程序 3。在运行时动态加载和链接共享库 libvector.so
~~~c
// dll.c
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int x[2] = {1, 2};
int y[2] = {3, 4};
int z[2];

int main()
{
    void *handle;
    void (*addvec)(int *, int *, int *, int);
    char *error;

    /* Dynamically load the shared library containing addvec() */
    handle = dlopen("./libvector.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    /* Get a pointer to the addvec() function we just loaded */
    addvec = dlsym(handle, "addvec");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }

    /* Now we can call addvec() just like any other function */
    addvec(x, y, z, 2);
    printf("z = [%d %d]\n", z[0], z[1]);

    /* Unload the shared library */
    if (dlclose(handle) < 0) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
    return 0;
}
~~~

这段代码展示了如何在 C 程序中动态加载和使用共享库（在这个例子中是 `libvector.so`）。
程序使用了 `dlfcn.h` 头文件中定义的函数来加载共享库、获取函数地址、调用函数，以及卸载共享库。让我们逐步解析代码的主要部分：

1. **定义全局数组**：
   ```c
   int x[2] = {1, 2};
   int y[2] = {3, 4};
   int z[2];
   ```
   这里定义了三个整型数组 `x`、`y` 和 `z`，用于存储操作数和结果。

2. **`main` 函数**：
   - `main` 函数是程序的入口点。

3. **加载共享库**：
   ```c
   handle = dlopen("./libvector.so", RTLD_LAZY);
   ```
   - 使用 `dlopen` 函数动态加载 `libvector.so` 库。`RTLD_LAZY` 表示使用延迟绑定，即函数地址会在第一次调用时解析。

4. **错误检查**：
   - 如果 `dlopen` 返回 `NULL`，则打印错误信息并退出。

5. **获取函数指针**：
   ```c
   addvec = dlsym(handle, "addvec");
   ```
   - 使用 `dlsym` 函数获取 `addvec` 函数的地址。`addvec` 函数预期在 `libvector.so` 中定义。

6. **再次错误检查**：
   - 如果 `dlsym` 返回错误，程序打印错误信息并退出。

7. **调用函数**：
   ```c
   addvec(x, y, z, 2);
   ```
   - 调用 `addvec` 函数，传递数组 `x`、`y` 和 `z`，以及元素数量（2）。

8. **打印结果**：
   - 打印 `z` 数组的内容，该数组是 `addvec` 函数的输出结果。

9. **卸载共享库**：
   ```c
   if (dlclose(handle) < 0) {
       fprintf(stderr, "%s\n", dlerror());
       exit(1);
   }
   ```
   - 使用 `dlclose` 卸载共享库并检查错误。

这个程序的关键特点在于它在运行时动态地加载和使用共享库。
这种方式的优点是程序在编译时不需要知道库的具体实现，增加了程序的灵活性。
例如，可以在不重新编译程序的情况下更新或替换共享库。这也是许多插件系统和模块化架构的基础。


> 旁注 - 共享库和 Java 本地接口
> Java 定义了一个标准调用规则，叫做 Java 本地接口（*Java Native Interface，JNI*），它允许 Java 程序调用“本地的” C 和 C++ 函数。
> JNI 的基本思想是将本地 C 函数（如 foo）编译到一个共享库中（如 foo.so）。 
> 当一个正在运行的 Java 程序试图调用函数 foo 时，Java 解释器利用 dlopen 接口（或者与其类似的接口）动态链接和加载 foo.so，然后再调用 foo。