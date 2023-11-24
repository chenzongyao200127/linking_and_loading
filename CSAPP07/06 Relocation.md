# Relocation

一旦链接器完成了符号解析这一步，就把代码中的每个符号引用和正好一个符号定义（即它的一个输入目标模块中的一个符号表条目）关联起来。
此时，链接器就知道它的输入目标模块中的代码节和数据节的确切大小。现在就可以开始重定位步骤了，在这个步骤中，将合并输入模块，并为每个符号分配运行时地址。重定位由两步组成：

 1. **重定位节和符号定义**。
 在这一步中，链接器将所有相同类型的节合并为同一类型的新的聚合节。
 例如，来自所有输入模块的`.data` 节被全部合并成一个节，这个节成为输出的可执行目标文件的`.data` 节。然后，链接器将运行时内存地址赋给新的聚合节，赋给输入模块定义的每个节，以及赋给输入模块定义的每个符号。当这一步完成时，程序中的每条指令和全局变量都有唯一的运行时内存地址了。

 2. **重定位节中的符号引用**。
 在这一步中，链接器修改代码节和数据节中对每个符号的引用，使得它们指向正确的运行时地址。
 要执行这一步，链接器依赖于可重定位目标模块中称为重定位条目（relocation entry）的数据结构，我们接下来将会描述这种数据结构。


## 重定位条目

当汇编器生成一个目标模块时，它并不知道数据和代码最终将放在内存中的什么位置。它也不知道这个模块引用的任何外部定义的函数或者全局变量的位置。所以，无论何时汇编器遇到对最终位置未知的目标引用，它就会生成一个重定位条目，告诉链接器在将目标文件合并成可执行文件时如何修改这个引用。代码的重定位条目放在 `.rel.text` 中。已初始化数据的重定位条目放在 `.rel.data` 中。

图 7-9 展示了 ELF 重定位条目的格式。
 - `offset` 是需要被修改的引用的节偏移。
 - `symbol` 标识被修改引用应该指向的符号。
 - `type` 告知链接器如何修改新的引用。
 - `addend` 是一个有符号常数，一些类型的重定位要使用它对被修改引用的值做偏移调整。

~~~c
typedef struct {
    long offset;    /* Offset of the reference to relocate */
    long type:32,   /* Relocation type */
         symbol:32; /* Symbol table index */
    long addend;    /* Constant part of relocation expression */
} Elf64_Rela;
~~~

ELF 定义了 32 种不同的重定位类型，有些相当隐秘。我们只关心其中两种最基本的重定位类型：
 - `R_X86_64_PC32`。
 重定位一个使用 32 位 PC 相对地址的引用。回想一下 3.6.3 节，一个 PC 相对地址就是距程序计数器（PC）的当前运行时值的偏移量。
 当 CPU 执行一条使用 PC 相对寻址的指令时，它就将在指令中编码的 32 位值加上 PC 的当前运行时值，得到有效地址（如 call 指令的目标），PC 值通常是下一条指令在内存中的地址。
 - `R_X86_64_32`。
 重定位一个使用 32 位绝对地址的引用。通过绝对寻址，CPU 直接使用在指令中编码的 32 位值作为有效地址，不需要进一步修改。

这两种重定位类型支持 x86-64 小型代码模型（small code model），该模型假设可执行目标文件中的代码和数据的总体大小小于 2GB，因此在运行时可以用 32 位 PC 相对地址来访问。GCC 默认使用小型代码模型。大于 2GB 的程序可以用 `-mcmodel=medium`（中型代码模型）和 `-mcmodel=large`（大型代码模型）标志来编译，不过在此我们不讨论这些模型。


## 重定位符号引用 ⭐

图 7-10 展示了链接器的重定位算法的伪代码。

~~~c
foreach section s {
    foreach relocation entry r {
        refptr = s + r.offset;  /* ptr to reference to be relocated */
    
        /* Relocate a PC-relative reference */
        if (r.type == R_X86_64_PC32){
            refaddr = ADDR(s) + r.offset; /* ref's run-time address */
            *refptr = (unsigned) (ADDR(r.symbol) + r.addend - refaddr);
        }

        /* Relocate an absolute reference */
        if (r.type ==R_X86_64_32)
            *refptr = (unsigned) (ADDR(r.symbol) + r.addend);
    }
}
~~~

The code snippet you've provided appears to be a part of a relocation algorithm used in binary or object file processing, specifically for an x86-64 architecture. It deals with adjusting addresses in a binary during the process of loading or linking.

Let's break down the key elements:

1. **`foreach section s`:** This line suggests iterating over each section of the binary or object file. Sections in a binary file typically include data, text (code), BSS (uninitialized data), etc.

2. **`foreach relocation entry r`:** Within each section, the code iterates over relocation entries. Relocation entries contain information necessary to adjust symbol addresses so that they point to the correct locations.

3. **`refptr = s + r.offset;`**: 
This line calculates a pointer to the location within the section where a relocation needs to be applied. It adds the offset of the relocation entry to the start of the section.


4. **Relocating PC-relative references (`R_X86_64_PC32`):**
    - **`refaddr = ADDR(s) + r.offset;`**: Calculates the runtime address of the reference.
    - **`*refptr = (unsigned) (ADDR(r.symbol) + r.addend - refaddr);`**
    The actual relocation is performed by adjusting the value at `refptr`. It's set to the address of the symbol (plus any additional offset, `r.addend`) minus the reference's own address. This is typical for PC-relative addressing, where instructions or data elements refer to other locations relative to their own address.
    实际的重定位是通过调整 refptr 处的值来完成的。它被设置为符号的地址（加上任何额外的偏移量，r.addend）减去引用自己的地址。这对于 PC-相对寻址是典型的，其中指令或数据元素相对于它们自己的地址引用其他位置。

5. **Relocating absolute references (`R_X86_64_32`):**
    - In this case, the value at `refptr` is simply set to the absolute address of the symbol plus the addend. This is for references that are absolute, not relative to the Program Counter (PC).
    在这种情况下，refptr 处的值简单地被设置为符号的绝对地址加上增量。这适用于绝对引用，而不是相对于程序计数器（PC）的引用。

In summary, this code is part of a linker or loader, specifically handling the relocation of symbols in an x86-64 architecture. The relocation process adjusts symbol references in the binary so they point to the correct memory addresses at runtime. The two main types of relocations handled here are PC-relative and absolute.


第 1 行和第 2 行在每个节 `s` 以及与每个节相关联的重定位条目 `r` 上迭代执行。
为了使描述具体化，假设每个节 `s` 是一个字节数组，每个重定位条目 `r` 是一个类型为 `Elf64_Rela` 的结构，如图 7-9 中的定义。
~~~c
typedef struct {
    long offset;    /* Offset of the reference to relocate */
    long type:32,   /* Relocation type */
         symbol:32; /* Symbol table index */
    long addend;    /* Constant part of relocation expression */
} Elf64_Rela;
~~~
另外，还假设当算法运行时，链接器已经为每个节（用 `ADDR(s)` 表示）和每个符号都选择了运行时地址（用 `ADDR(r.symbol)` 表示）。
第 3 行计算的是需要被重定位的 4 字节引用的数组 s 中的地址。
如果这个引用使用的是 PC 相对寻址，那么它就用第 5 ~ 9 行来重定位。
如果该引用使用的是绝对寻址，它就通过第 11 ~ 13 行来重定位。


为了更具体地解释这段代码的作用，让我们创建一个简单的实际例子。假设我们有一个小程序，它由两个部分组成：代码段和数据段。
代码段中包含对数据段中变量的引用。这种情况在编译和链接时很常见。

### 示例场景设置
- **数据段**（Data Section）: 假设包含一个变量 `var_data`，在编译时其地址未知。
- **代码段**（Code Section）: 包含一条指令，引用 `var_data`。

### 编译和链接
1. **编译过程**:
   - 编译器处理这些段，并且在发现代码段中对数据段中变量的引用时，它创建一个重定位条目。
   - 这个条目包含如何修改代码段中的引用以指向正确地址的信息。

2. **链接过程**:
   - 链接器读取这些重定位条目，并根据最终确定的内存地址来调整代码段中的引用。

### 重定位过程的代码实现
假设我们的代码和数据如下所示：

```c
// 数据段
int var_data = 42;

// 代码段（伪代码）
int main() {
    int x = var_data;  // 引用 var_data
}
```

在编译过程中，编译器会生成一个重定位条目，表示 `main` 函数中的 `var_data` 引用需要被修正以指向正确的地址。

在链接阶段，链接器使用你提供的代码进行重定位：

```c
foreach section s {
    foreach relocation entry r {
        refptr = s + r.offset;  // 指向需要重定位的引用

        // 处理 PC-相对引用
        if (r.type == R_X86_64_PC32){
            refaddr = ADDR(s) + r.offset;  // 引用的运行时地址
            *refptr = (unsigned) (ADDR(r.symbol) + r.addend - refaddr);
        }

        // 处理绝对引用
        if (r.type == R_X86_64_32)
            *refptr = (unsigned) (ADDR(r.symbol) + r.addend);
    }
}
```

在这个例子中，`var_data` 的引用（在 `main` 函数中）将通过修改 `refptr` 来重定位。
如果这个引用是 PC-相对的，那么它将根据代码段和数据段的实际运行时地址进行调整。如果它是绝对引用，那么它将直接被设置为 `var_data` 的运行时地址。

### 总结
在实际的应用中，这段代码会根据编译时生成的重定位条目来调整代码段中的引用，确保它们在程序运行时指向正确的内存地址。
这是二进制文件的加载和链接过程的重要部分，特别是在涉及动态链接的情况下更为关键。



让我们来看看链接器如何用这个算法来重定位图 7-1 示例程序中的引用。
图 7-11 给出了（用 objdump-dx main.o 产生的）GNU OBJDUMP 工具产生的 main.o 的反汇编代码。

~~~c
int sum(int *a, int n);

int array[2] = {1, 2};

int main()
{
    int val = sum(array, 2);
    return val;
}
~~~

~~~shell
Disassembly of section .text:

0000000000000000 <main>:
   0:   f3 0f 1e fa             endbr64 
   4:   55                      push   %rbp
   5:   48 89 e5                mov    %rsp,%rbp
   8:   48 83 ec 10             sub    $0x10,%rsp
   c:   be 02 00 00 00          mov    $0x2,%esi
  11:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 18 <main+0x18>
                        14: R_X86_64_PC32       array-0x4
  18:   e8 00 00 00 00          callq  1d <main+0x1d>
                        19: R_X86_64_PLT32      sum-0x4
  1d:   89 45 fc                mov    %eax,-0x4(%rbp)
  20:   8b 45 fc                mov    -0x4(%rbp),%eax
  23:   c9                      leaveq 
  24:   c3                      retq 
~~~


`main` 函数引用了两个全局符号：`array` 和 `sum`。
为每个引用，汇编器产生一个重定位条目，显示在引用的后面一行上。
这些重定位条目告诉链接器对 sum 的引用要使用 32 位 PC 相对地址进行重定位，而对 array 的引用要使用 32 位绝对地址进行重定位。
接下来两节会详细介绍链接器是如何重定位这些引用的。
> 回想一下. 重定位条目和指令实际上存放在目标文件的不同节中。为了方便，OBJDUMP 工具把它们显示在一起。

====================================================================================================
### 汇编代码解析
1. **引用 `array`**:
   - 在地址 `0x11` 处，有一条 `lea` 指令，它加载了相对于指令指针（`%rip`）的 `array` 的地址。
   - 重定位条目 `14: R_X86_64_PC32 array-0x4` 表示这是一个 PC-相对的 32 位地址。这意味着链接器需要调整这个地址，使其正确地指向 `array` 的运行时地址。

2. **引用 `sum`**:
   - 在地址 `0x18` 处，有一条 `callq` 指令，它调用 `sum` 函数。
   - 重定位条目 `19: R_X86_64_PLT32 sum-0x4` 表示这是一个通过过程链接表（Procedure Linkage Table, PLT）的调用，同样是 PC-相对的。

### 链接器的工作
对于这两个引用，链接器的任务是根据最终确定的符号地址来调整指令中的地址。具体来说：

1. **对于 `array` 的引用**:
   - 链接器会计算 `main` 函数的运行时地址加上 `lea` 指令后面的偏移量（从重定位条目获取），然后根据 `array` 符号的最终地址调整这个偏移量。

2. **对于 `sum` 的调用**:
   - 链接器会修改 `callq` 指令，以便它跳转到 `sum` 函数的正确地址。这通常通过修改 PLT 来实现，PLT 是动态链接时用于解决函数地址的表格。
====================================================================================================

~~~powershell
# example
0000000000000000 <main>:         
   0:   48 83 ec 08             sub    $0x8,%rsp
   4:   be 02 00 00 00          mov    $0x2,%esi
   9:   bf 00 00 00 00          mov    $0x0,%edi        %edi = &array
                        a: R_X86_64_32 array            Relocation entry
   e:   e8 00 00 00 00          callq  13 <main+0x13>   sum()
                        f: R_X86_64_PC32 sum-0x4        Relocation entry
  13:   48 83 c4 08             add $0x8,%rsp
  17:   c3                      retq
~~~

1. 重定位 PC 相对引用
图 7-11 的第 6 行中，函数 main 调用 sum 函数，sum 函数是在模块 sum.o 中定义的。
call 指令开始于节偏移 0xe 的地方’包括 1 字节的操作码 0xe8，后面跟着的是对目标 sum 的 32 位 PC 相对引用的占位符。
相应的重定位条目 r 由 4 个字段组成：

~~~c
r.offset = 0xf
r.symbol = sum
r.type   = R_X86_64_PC32
r.addend = -4
~~~

这些字段告诉链接器修改开始于偏移量 `0xf` 处的 32 位 PC 相对引用，这样在运行时它会指向 `sum` 例程。
现在，假设链接器已经确定 `ADDR(s) = ADDR(.text) = 0x4004d0` 和 `ADDR(r.symbol) = ADDR(sum) = 0x4004e8`
使用图 7-10 中的算法，链接器首先计算出引用的运行时地址（第 7 行）：

~~~c
refaddr = ADDR(s)  + r.offset
        = 0x4004d0 + 0xf
        = 0x4004df
~~~

然后，更新该引用，使得它在运行时指向 sum 程序（第 8 行）：

~~~c
*refptr = (unsigned) (ADDR(r.symbol) + r.addend - refaddr)
        = (unsigned) (0x4004e8       + (-4)     - 0x4004df)
        = (unsigned) (0x5)
~~~

在得到的可执行目标文件中，call 指令有如下的重定位的形式：
~~~shell
4004de:  e8 05 00 00 00      callq  4004e8 <sum>       sum()
~~~

在 call 指令之后的指令的地址。为了执行这条指令，CPU 执行以下的步骤：

1）将 PC 压入栈中
2）`PC ← PC + 0x5` = `0x4004e3 + 0x5 = 0x4004e8`

因此，要执行的下一条指令就是 sum 例程的第一条指令，这当然就是我们想要的！



# 重定位绝对引用

重定位绝对引用相当简单。
例如，图 7-11 的第 4 行中
~~~shell
9:   bf 00 00 00 00          mov    $0x0,%edi        %edi = &array
                        a: R_X86_64_32 array            Relocation entry
~~~

mov 指令将 array 的地址（一个 32 位立即数值）复制到寄存器`％edi` 中。
mov 指令开始于节偏移量 0x9 的位置，包括 1 字节操作码 Oxbf，后面跟着对 array 的 32 位绝对引用的占位符。

对应的占位符条目 r 包括 4 个字段：
~~~shell
r.offset = 0xa
r.symbol = array
r.type   = R_X86_64_32
r.addend = 0
~~~
这些字段告诉链接器要修改从偏移量 `0xa` 开始的绝对引用，这样在运行时它将会指向 array 的第一个字节。
现在，假设链接器巳经确定
`ADDR(r.symbol) = ADDR(array) = 0x601018`
链接器使用图 7-10 中算法的第 13 行修改了引用：
~~~c
*refptr = (unsigned) (ADDR(r.symbol) + r.addend)
        = (unsigned) (0x601018       + 0)
        = (unsigned) (0x601018)
~~~

在得到的可执行目标文件中，该引用有下面的重定位形式：
~~~shell
4004d9:  bf 18 10 60 00	      mov   $0x601018,%edi         %edi = &array
~~~

综合到一起，图 7-12 给出了最终可执行目标文件中已重定位的 .text 节和 .data 节。
在加载的时候，加载器会把这些节中的字节直接复制到内存，不再进行任何修改地执行这些指令。
~~~shell
00000000004004d0 <main>:
  4004d0:  48 83 ec 08          sub    $0x8,%rsp
  4004d4:  be 02 00 00 00       mov    $0x2,%esi
  4004d9:  bf 18 10 60 00       mov    $0x601018,%edi    %edi = &array
  4004de:  e8 05 00 00 00       callq  4004e8 <sum>      sum()
  4004e3:  48 83 c4 08          add    $0x8,%rsp
  4004e7:  c3                   retq

00000000004004e8 <sum>:
  4004e8:  b8 00 00 00 00       mov    $0x0,%eax
  4004ed:  ba 00 00 00 00       mov    $0x0,%edx
  4004f2:  eb 09                jmp    4004fd <sum+0x15>
  4004f4:  48 63 ca             movslq %edx,%rcx
  4004f7:  03 04 8f             add    (%rdi,%rcx,4),%eax
  4004fa:  83 c2 01             add    $0x1,%edx
  4004fd:  39 f2                cmp    %esi,%edx
  4004ff:  7c f3                jl     4004f4 <sum+0xc>
  400501:  f3 c3                repz retq
~~~

~~~shell
0000000000601018 <array>:
  601018:  01 00 00 00 02 00 00 00
~~~

本题是关于图 7-12a 中的已重定位程序的。
A. 第 5 行中对 sum 的重定位引用的十六进制地址是多少？ 
`0x4004df`
B. 第 5 行中对 sum 的重定位引用的十六进制值是多少？
`0x5`

这道题涉及的是图 7-12a 中的反汇编列表。目的是让你练习阅读反汇编列表，并检查你对 PC 相对寻址的理解。
A. 第 5 行被重定位引用的十六进制地址为 0x4004df。
B. 第 5 行被重定位引用的十六进制值为 0x5。记住，反汇编列表给岀的引用值是用小端法字节顺序表示的。


考虑目标文件 m.o 中对 swap 函数的调用（图 7-5）。
~~~shell
9:  e8 00 00 00 00            callq  e <main+0xe>      swap()
~~~
它的重定位条目如下：
~~~shell
r.offset = 0xa
r.symbol = swap
r.type   = R_X86_64_PC32
r.addend = -4
~~~
现在假设链接器将 m.o 中的 .text 重定位到地址 0x4004d0，将 swap 重定位到地址 0x4004e8。那么 callq 指令中对 swap 的重定位引用的值是什么？

`0x4004e8 + (-4) - (0x4004d0 + 0xa) ` = 10 (0xa)
