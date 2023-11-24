# Library Interpositioning
# 库打桩机制

Linux 链接器支持一个很强大的技术，称为库打桩（library interpositioning），它允许你截获对共享库函数的调用，取而代之执行自己的代码。使用打桩机制，你可以追踪对某个特殊库函数的调用次数，验证和追踪它的输入和输出值，或者甚至把它替换成一个完全不同的实现。

下面是它的基本思想：给定一个需要打桩的目标函数，创建一个包装函数，它的原型与目标函数完全一样。
使用某种特殊的打桩机制，你就可以欺骗系统调用包装函数而不是目标函数了。包装函数通常会执行它自己的逻辑，然后调用目标函数，再将目标函数的返回值传递给调用者。

打桩可以发生在编译时、链接时或当程序被加载和执行的运行时。
要研究这些不同的机制，我们以图 7-20a 中的示例程序作为运行例子。它调用 C 标准库（libc.so）中的 malloc 和 free 函数。对 malloc 的调用从堆中分配一个 32 字节的块，并返回指向该块的指针。对 free 的调用把块还回到堆，供后续的 malloc 调用使用。
我们的目标是用打桩来追踪程序运行时对 malloc 和 free 的调用。

# 编译时打桩

图 7-20 展示了如何使用 C 预处理器在编译时打桩。
mymalloc.c 中的包装函数（图 7-20c）调用目标函数，打印追踪记录，并返回。
本地的 malloc.h 头文件（图 7-20b）指示预处理器用对相应包装函数的调用替换掉对目标函数的调用。

~~~c
// int.c
#include <stdio.h>
#include <malloc.h>

int main()
{
    int *p = malloc(32);
    free(p);
    return(0);
}

// malloc.h
#define malloc(size) mymalloc(size)
#define free(ptr) myfree(ptr)

void *mymalloc(size_t size);
void myfree(void *ptr);

// mymalloc.h
#ifdef COMPILETIME
#include <stdio.h>
#include <malloc.h>

/* malloc wrapper function */
void *mymalloc(size_t size)
{
    void *ptr = malloc(size);
    printf("malloc(%d)=%p\n",
           (int)size, ptr);
    return ptr;
}

/* free wrapper function */
void myfree(void *ptr)
{
    free(ptr);
    printf("free(%p)\n", ptr);
}
#endif
~~~

用 C 预处理进行编译时打桩
像下面这样编译和链接这个程序：

~~~shell
czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± gcc -DCOMPILETIME -c mymalloc.c                                                                          15:25

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± gcc -I. -o intc int.c mymalloc.o                                                                         15:25

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± ls                                                                                                       15:25
int.c  intc  malloc.h  mymalloc.c  mymalloc.o

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± ./intc                                                                                                   15:26
malloc(32)=0x5580916292a0
free(0x5580916292a0)
~~~

这个例子展示了如何使用 C 预处理器和一些编译技巧来实现编译时打桩（Compile-time Interpositioning）。
打桩是一种软件工程技术，用于监控和/或改变程序中特定函数的行为。
在这个例子中，对 `malloc` 和 `free` 函数的调用被重定向到自定义的 `mymalloc` 和 `myfree` 函数，以便在分配和释放内存时打印相关信息。

### 代码解析

1. **`int.c`**:
   - 这是主程序文件。它包含了对 `malloc` 和 `free` 函数的调用。

2. **`malloc.h`**:
   - 这是一个头文件，使用宏定义将 `malloc` 和 `free` 的调用重定向到 `mymalloc` 和 `myfree`。
   - 当 `int.c` 包含这个头文件时，所有的 `malloc` 和 `free` 调用都会变成 `mymalloc` 和 `myfree` 的调用。

3. **`mymalloc.h`**:
   - 这个头文件定义了 `mymalloc` 和 `myfree` 函数。
   - 它们分别封装了标准的 `malloc` 和 `free` 函数，并在调用这些函数时打印出内存分配和释放的信息。

4. **编译和链接**:
   - `gcc -DCOMPILETIME -c mymalloc.c`：这个命令编译 `mymalloc.c` 文件，并通过 `-DCOMPILETIME` 定义了 `COMPILETIME` 宏，确保只包含适当的代码。
   - `gcc -I. -o intc int.c mymalloc.o`：这个命令编译 `int.c` 并链接之前编译的 `mymalloc.o` 对象文件，生成可执行文件 `intc`。

### 运行结果

- 当运行 `./intc` 时，输出如下：
  - `malloc(32)=0x5580916292a0`
  - `free(0x5580916292a0)`
- 这表示 `malloc` 被成功地替换为 `mymalloc`，`free` 被替换为 `myfree`，并且在调用这些函数时输出了内存分配和释放的信息。

这种方法的优势在于能够不修改现有代码的情况下，监控或改变特定函数的行为。这在调试、性能监控或者实现特定的运行时特性时非常有用。


# 链接时打桩
Linux 静态链接器支持用 `--wrap` `f` 标志进行链接时打桩。这个标志告诉链接器，把对符号 `f` 的引用解析成 `__wrap_f`（前缀是两个下划线），还要把对符号 `__real_f`（前缀是两个下划线）的引用解析为 `f`。图 7-21 给出我们示例程序的包装函数。

~~~c
#ifdef LINKTIME
#include <stdio.h>

void *__real_malloc(size_t size);
void __real_free(void *ptr);

/* malloc wrapper function */
void *__wrap_malloc(size_t size)
{
    void *ptr = __real_malloc(size); /* Call libc malloc */
    printf("malloc(%d) = %p\n", (int)size, ptr);
    return ptr;
}

/* free wrapper function */
void __wrap_free(void *ptr)
{
    __real_free(ptr); /* Call libc free */
    printf("free(%p)\n", ptr);
}
#endif
~~~

用 --wrap 标志进行链接时打桩
用下述方法把这些源文件编译成可重定位目标文件

~~~shell
czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± gcc -DLINKTIME -c mymalloc.c                                                                             15:26

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± gcc -c int.c                                                                                             15:30

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± gcc -Wl,--wrap,malloc -Wl,--wrap,free -o intl int.o mymalloc.o                                           15:30

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± ./intl                                                                                                   15:30
malloc(32) = 0x55c88d4042a0
free(0x55c88d4042a0)
~~~

`-Wl,option` 标志把 option 传递给链接器。option 中的每个逗号都要替换为一个空格。所以 -Wl,--wrap,malloc 就把 --wrap malloc 传递给链接器，以类似的方式传递 -Wl,--wrap,free。


这段代码是一个使用链接时打桩（Link-time Interpositioning）的例子。
链接时打桩是一种在编译和链接阶段重定向函数调用的技术。在这个例子中，`malloc` 和 `free` 函数的调用被重定向到自定义的 `__wrap_malloc` 和 `__wrap_free` 函数。这种重定向是通过链接器的 `--wrap` 选项实现的。

1. **宏定义检查**:
   - `#ifdef LINKTIME`：这个预处理指令检查是否定义了 `LINKTIME` 宏。如果定义了，以下代码才会被包含在编译中。

2. **自定义的包装函数**:
   - `__wrap_malloc` 和 `__wrap_free` 是对 `malloc` 和 `free` 的包装函数。
   - 这些函数在调用实际的 `malloc`（通过 `__real_malloc`）和 `free`（通过 `__real_free`）之前和之后执行额外的操作（例如打印信息）。

3. **调用实际的库函数**:
   - `__real_malloc` 和 `__real_free` 是链接器提供的特殊符号，它们指向原始的 `malloc` 和 `free` 函数。
   - 在这些包装函数中，首先调用原始的库函数，然后执行额外的操作（如打印）。

### 编译和链接

为了使用这些包装函数，需要在编译和链接时进行特殊处理：

- 编译时，包含这段代码的文件需要被编译成对象文件。
- 链接时，使用链接器的 `--wrap` 选项。例如，使用 `gcc -Wl,--wrap=malloc -Wl,--wrap=free ...` 来链接程序。这告诉链接器将对 `malloc` 的调用重定向到 `__wrap_malloc`，将对 `free` 的调用重定向到 `__wrap_free`。

### 使用场景

链接时打桩的技术常用于测试、调试，或者在不改变源代码的情况下改变某些函数的行为。例如，可以用于监控内存分配和释放，检测内存泄漏，或者添加自定义的内存管理逻辑。



# 运行时打桩
编译时打桩需要能够访问程序的源代码，链接时打桩需要能够访问程序的可重定位对象文件。
不过，有一种机制能够在运行时打桩，它只需要能够访问可执行目标文件。这个很厉害的机制基于动态链接器的 `LD_PRELOAD` 环境变量。

如果 `LD_PRELOAD` 环境变量被设置为一个共享库路径名的列表（以空格或分号分隔），那么当你加载和执行一个程序，需要解析未定义的引用时，动态链接器（`LD-LINUX.SO`）会先搜索 `LD_PRELOAD` 库，然后才搜索任何其他的库。
有了这个机制，当你加载和执行任意可执行文件时，可以对任何共享库中的任何函数打桩，包括 `libc.so`。

图 7-22 展示了 malloc 和 free 的包装函数。每个包装函数中，对 `dlsym` 的调用返回指向目标 libc 函数的指针。然后包装函数调用目标函数，打印追踪记录，再返回。

~~~c
#ifdef RUNTIME
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

/* malloc wrapper function */
void *malloc(size_t size)
{
    void *(*mallocp)(size_t size);
    char *error;

    mallocp = dlsym(RTLD_NEXT, "malloc"); /* Get address of libc   malloc */ 
    if ((error = dlerror()) != NULL) { 
        fputs(error, stderr);
        exit(1);
    }
    char *ptr = mallocp(size); /* Call libc malloc */
    printf("malloc(%d) = %p\n", (int)size, ptr);
    return ptr;
}

/* free wrapper function */
void free(void *ptr)
{
    void (*freep)(void *) = NULL;
    char *error;

    if (!ptr)
    return;

    freep = dlsym(RTLD_NEXT, "free"); /* Get address of libc free */
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    freep(ptr); /* Call libc free */
    printf("free(%p)\n", ptr);
}
#endif
~~~

下面是如何构建包含这些包装函数的共享库的方法：
~~~shell
czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± gcc -DRUNTIME -shared -fpic -o mymalloc.so mymalloc.c -ldl                                               15:35

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± gcc -o intr int.c                                                                                        15:35

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± LD_PRELOAD="./mymalloc.so" ./intr       
                                                                 15:35
malloc(32) = 0x55f582e862a0
free(0x55f582e862a0)


!!! 你可以用 LD_PRELOAD 对任何可执行程序的库函数调用打桩！

czy in ~/new_space/linking_and_loading/CSAPP07/12 on master !
± LD_PRELOAD="./mymalloc.so" /usr/bin/uptime dust                                                          15:35
malloc(37) = 0x55a0902f52d0
malloc(472) = 0x55a0902f5300
malloc(4096) = 0x55a0902f54e0
malloc(1024) = 0x55a0902f64f0
free(0x55a0902f64f0)
free(0x55a0902f5300)
free(0x55a0902f54e0)
malloc(472) = 0x55a0902f5300
malloc(1024) = 0x55a0902f64f0
free(0x55a0902f64f0)
free(0x55a0902f5300)
malloc(472) = 0x55a0902f5300
malloc(4096) = 0x55a0902f54e0
malloc(1024) = 0x55a0902f64f0
free(0x55a0902f64f0)
free(0x55a0902f5300)
free(0x55a0902f54e0)
malloc(5) = 0x55a0902f54e0
free(0x55a0902f54e0)
malloc(120) = 0x55a0902f5500
malloc(12) = 0x55a0902f54e0
malloc(784) = 0x55a0902f5580
malloc(112) = 0x55a0902f58a0
malloc(1336) = 0x55a0902f5920
malloc(216) = 0x55a0902f5e60
malloc(432) = 0x55a0902f5f40
malloc(104) = 0x55a0902f6100
malloc(88) = 0x55a0902f6170
malloc(120) = 0x55a0902f61d0
malloc(168) = 0x55a0902f6250
malloc(104) = 0x55a0902f6300
malloc(80) = 0x55a0902f6370
malloc(192) = 0x55a0902f63d0
malloc(12) = 0x55a0902f64a0
malloc(12) = 0x55a0902f64c0
malloc(12) = 0x55a0902f6900
malloc(12) = 0x55a0902f6920
malloc(12) = 0x55a0902f6940
malloc(12) = 0x55a0902f6960
malloc(5) = 0x55a0902f6980
free(0x55a0902f6980)
malloc(120) = 0x55a0902f69a0
malloc(11) = 0x55a0902f6980
malloc(784) = 0x55a0902f6a20
malloc(112) = 0x55a0902f6d40
malloc(1336) = 0x55a0902f6dc0
malloc(216) = 0x55a0902f7300
malloc(432) = 0x55a0902f73e0
malloc(104) = 0x55a0902f75a0
malloc(88) = 0x55a0902f7610
malloc(120) = 0x55a0902f7670
malloc(168) = 0x55a0902f76f0
malloc(104) = 0x55a0902f77a0
malloc(80) = 0x55a0902f7810
malloc(192) = 0x55a0902f7870
malloc(11) = 0x55a0902f7940
malloc(12) = 0x55a0902f7960
malloc(11) = 0x55a0902f7980
malloc(12) = 0x55a0902f79a0
malloc(12) = 0x55a0902f79c0
malloc(11) = 0x55a0902f79e0
malloc(278) = 0x55a0902f7a00
free((nil))
free((nil))
malloc(34) = 0x55a0902f7b20
malloc(10) = 0x55a0902f7b50
free((nil))
malloc(15) = 0x55a0902f7b70
malloc(472) = 0x55a0902f5300
free((nil))
malloc(4096) = 0x55a0902f7b90
malloc(303) = 0x55a0902f8ba0
free(0x55a0902f7b90)
free(0x55a0902f5300)
malloc(20) = 0x55a0902f7b90
malloc(20) = 0x55a0902f7bb0
malloc(20) = 0x55a0902f7bd0
free((nil))
malloc(12) = 0x55a0902f7bf0
malloc(268) = 0x55a0902f7c10
free(0x55a0902f79c0)
free(0x55a0902f7a00)
free((nil))
malloc(12) = 0x55a0902f79c0
malloc(278) = 0x55a0902f7a00
free(0x55a0902f7c10)
free((nil))
free(0x55a0902f7bf0)
malloc(384) = 0x55a0902f7d30
malloc(12) = 0x55a0902f7bf0
malloc(268) = 0x55a0902f7c10
free(0x55a0902f79c0)
free(0x55a0902f7a00)
free((nil))
malloc(12) = 0x55a0902f79c0
malloc(278) = 0x55a0902f7a00
free(0x55a0902f7c10)
free((nil))
free(0x55a0902f7bf0)
malloc(1024) = 0x55a0902f64f0
 15:36:03 up 3 days,  1:17,  1 user,  load average: 0.05, 0.03, 0.00
free(0x55a0902f64f0)
~~~

这段代码展示了运行时打桩（Runtime Interpositioning）的一个例子，其中 `malloc` 和 `free` 函数被自定义的实现所替代。
这种替代是在程序运行时进行的，通过使用动态链接库（Dynamic Linking Library，DLL）提供的功能实现。

### 代码解析

1. **宏定义检查**:
   - `#ifdef RUNTIME` 检查是否定义了 `RUNTIME` 宏。如果定义了，以下代码才会被包含在编译中。

2. **自定义的 `malloc` 和 `free` 函数**:
   - 这些函数重写了标准的 `malloc` 和 `free` 函数。
   - 它们使用 `dlsym` 函数来动态地查找并调用真正的（原始的 libc 中的）`malloc` 和 `free` 函数。

3. **使用 `dlsym` 进行符号解析**:
   - `dlsym(RTLD_NEXT, "malloc")` 和 `dlsym(RTLD_NEXT, "free")` 调用动态查找下一个符号表中与 `malloc` 和 `free` 名称匹配的函数。
   - `RTLD_NEXT` 指定在当前库之后的符号表中查找，这通常意味着在 libc 中查找。

4. **错误检查**:
   - 如果 `dlsym` 返回错误，程序打印错误消息并退出。

5. **调用原始的库函数并打印信息**:
   - 在调用原始的 `malloc` 和 `free` 函数之后，自定义函数打印出相关信息。

### 编译和使用

要使用这些打桩函数，需要将它们编译为一个共享库，然后在运行时通过环境变量（如 `LD_PRELOAD`）将其加载到程序中。例如：

- 将这段代码编译为共享库：`gcc -fPIC -shared -o mymalloc.so this_code.c`
- 使用 `LD_PRELOAD` 来运行程序：`LD_PRELOAD=./mymalloc.so ./your_program`