7.5 符号和符号表

`.symtab`

每个可重定位目标模块 m 都有一个符号表，它包含 m 定义和引用的符号的信息。在链接器的上下文中，有三种不同的符号：

 - 由模块 m 定义并能被其他模块引用的全局符号。全局链接器符号对应于非静态的 C 函数和全局变量。

 - 由其他模块定义并被模块 m 引用的全局符号。这些符号称为外部符号，对应于在其他模块中定义的非静态 C 函数和全局变量。

 - 只被模块 m 定义和引用的局部符号。它们对应于带 static 属性的 C 函数和全局变量。这些符号在模块 m 中任何位置都可见，但是不能被其他模块引用。

认识到本地链接器符号和本地程序变量不同是很重要的。
`.symtab` 中的符号表不包含对应于本地非静态程序变量的任何符号。这些符号在运行时在栈中被管理，链接器对此类符号不感兴趣。

有趣的是，定义为带有 C static 属性的本地过程变量是不在栈中管理的。相反，编译器在 `.data` 或 `.bss` 中为每个定义分配空间，并在符号表中创建一个有唯一名字的本地链接器符号。 比如，假设在同一模块中的两个函数各自定义了一个静态局部变量 x：

~~~c
int f()
{
    static int x = 0;
    return x;
}

int g()
{
    static int x = 1;
    return x;
}
~~~

在这种情况中，编译器向汇编器输出两个不同名字的局部链接器符号。比如，它可以用 x.1 表示函数 f 中的定义，而用 x.2 表示函数 g 中的定义。

> 给 C 语言初学者 - 利用 static 属性隐藏变量和函数名字
> C 程序员使用 static 属性隐藏模块内部的变量和函数声明，就像你在 Java 和 C++ 中使用 public 和 private 声明一样。
> 在 C 中，源文件扮演模块的角色。任何带有 static 属性声明的全局变量或者函数都是模块私有的。
> 类似地，任何不带 static 属性声明的全局变量和函数都是公共的，可以被其他模块访问。尽可能用 static 属性来保护你的变量和函数是很好的编程习惯。 

符号表是由汇编器构造的，使用编译器输出到汇编语言 .s 文件中的符号。.symtab 节中包含 ELF 符号表。这张符号表包含一个条目的数组。图 7-4 展示了每个条目的格式。

~~~c
typedef struct {
    int     name;      /* String table offset */
    char    type:4,    /* Function or data (4 bits) */
            binding:4; /* Local or global (4 bits) */
    char    reserved;  /* Unused */
    short   section;   /* Section header index */
    long    value;     /* Section offset or absolute address */
    long    size;      /* Object size in bytes */
} Elf64_Symbol;
~~~


这个结构体`Elf64_Symbol`定义了一个符号表项（Symbol Table Entry）的属性。
符号表在ELF文件中是一个关键部分，它包含了关于程序中各种符号（如函数、变量等）的信息。下面是对该结构体各个字段的解释：

### 字段解释
1. **`int name;`**
   - 这是一个偏移量，指向字符串表（String Table），用于找到符号的名称。
2. **`char type:4;`**
   - 这是一个4位字段，定义了符号的类型。通常，它表示该符号是函数还是数据。
3. **`char binding:4;`**
   - 这同样是一个4位字段，定义了符号的绑定属性。它表明该符号是局部符号（Local）还是全局符号（Global）。
4. **`char reserved;`**
   - 这是一个保留字段，目前没有使用。
5. **`short section;`**
   - 这表示符号所在的节（Section）的索引。特殊值如 `SHN_UNDEF`（0）表示未定义的符号。
6. **`long value;`**
   - 对于数据类型的符号，这通常是符号在其节中的偏移量。对于函数类型的符号，它可能是函数的入口点。
7. **`long size;`**
   - 符号的大小（以字节为单位）。对于变量，它是变量的大小；对于函数，它通常是函数的长度。

### 注意事项
- 在64位系统上，`long`通常是64位的，而`int`和`short`分别是32位和16位。这与特定的平台和编译器有关。
- 字段中的位字段（`type`和`binding`）允许结构体在存储时更加紧凑。
- `reserved`字段在当前的ELF规范中保留未用，这为将来可能的扩展留下了空间。
- 符号表是链接和调试过程中非常重要的部分，它为链接器提供了必要的信息来正确地解析和链接程序中的符号。

这个结构体是理解和操作ELF文件中符号表的基础，特别是在涉及底层系统编程、链接器和加载器的实现时非常重要。


=========================== Too much detailed ==================================
`name` 是字符串表中的字节偏移，指向符号的以 null 结尾的字符串名字。
`value` 是符号的地址。对于可重定位的模块来说，value 是距定义目标的节的起始位置的偏移。对于可执行目标文件来说，该值是一个绝对运行时地址。
`size` 是目标的大小（以字节为单位）。
`type` 通常要么是数据，要么是函数。符号表还可以包含各个节的条目，以及对应原始源文件的路径名的条目。所以这些目标的类型也有所不同。
`binding` 字段表示符号是本地的还是全局的。

每个符号都被分配到目标文件的某个节，由 `section` 字段表示，该字段也是一个到节头部表的索引。
有三个特殊的伪节（pseudosection），它们在节头部表中是没有条目的：
 - ABS 代表不该被重定位的符号；
 - UNDEF 代表未定义的符号，也就是在本目标模块中引用，但是却在其他地方定义的符号；
 - COMMON 表示还未被分配位置的未初始化的数据目标。对于 COMMON 符号，value 字段给出对齐要求，而 size 给出最小的大小。
 注意，只有可重定位目标文件中才有这些伪节，可执行目标文件中是没有的。
COMMON 和 .bss 的区别很细微。现代的 GCC 版本根据以下规则来将可重定位目标文件中的符号分配到 COMMON 和 .bss 中：

`COMMON` - 未初始化的全局变量
`.bss`   - 未初始化的静态变量，以及初始化为 0 的全局或静态变量 

釆用这种看上去很绝对的区分方式的原因来自于链接器执行符号解析的方式，我们会在 7.6 节中加以解释。
================================================================================


GNU READELF 程序是一个査看目标文件内容的很方便的工具。
比如，下面是图 7-1 中示例程序的可重定位目标文件 main.o 的符号表中的最后三个条目。开始的 8 个条目没有显示出来，它们是链接器内部使用的局部符号。

/home/czy/new_space/linking_and_loading/CSAPP07/04/04.jpg



 - 在这个例子中，我们看到全局符号 main 定义的条目，它是一个位于 .text 节中偏移量为 0（即 value 值）处的 24 字节函数。
 - 其后跟随着的是全局符号 array 的定义，它是一个位于 .data 节中偏移量为 0 处的 8 字节目标。
 - 最后一个条目来自对外部符号 sum 的引用。
 READELF 用一个整数索引来标识每个节。Ndx=1 表示 .text 节，而 Ndx=3 表示 .data 节。




`练习题 7.1`
这个题目针对图 7-5 中的 m.o 和 swap.o 模块。
对于每个在 swap.o 中定义或引用的符号，请指出它是否在模块 swap.o 中的 .symtab 节中有一个符号表条目。
如果是，请指出定义该符号的模块（swap.o 或者 m.o）、符号类型（局部、全局或者外部）以及它在模块中被分配到的节（.text、.data、.bss 或 COMMON）。

        .symtab entry?  Symbol type     Module where defined    Section
buf     v               外部                m.o                  .data   
bufp0   v               全局                swap.o               .data   
bufp1   v               全局                swap.o               COMMON
swap    v               全局                swap.o               .text
temp    x


~~~c
// m.c
void swap();

int buf[2] = {1, 2};

int main()
{
    swap();
    return 0;
}

// swap.c
extern int buf[];

int *bufp0 = &buf[0];
int *bufp1;

void swap()
{
    int temp;
    
    bufp1 = &buf[1];
    temp = *bufp0;
    *bufp0 = *bufp1;
    *bufp1 = temp;
}
~~~
夹在 ELF 头和节头部表之间的都是节。一个典型的 ELF 可重定位目标文件包含下面几个节： 
`.text`：已编译程序的机器代码。
`.rodata`：只读数据，比如 printf 语句中的格式串和开关语句的跳转表。
`.data`：已初始化的全局和静态 C 变量。局部 C 变量在运行时被保存在栈中，既不岀现在 .data 节中，也不岀现在 .bss 节中。
`.bss`：未初始化的全局和静态 C 变量，以及所有被初始化为 0 的全局或静态变量。
        在目标文件中这个节不占据实际的空间，它仅仅是一个占位符。目标文件格式区分已初始化和未初始化变量是为了空间效率：
        在目标文件中，未初始化变量不需要占据任何实际的磁盘空间。运行时，在内存中分配这些变量，初始值为 0。
`.symtab`：一个符号表，它存放在程序中定义和引用的函数和全局变量的信息。
        一些程序员错误地认为必须通过 -g 选项来编译一个程序，才能得到符号表信息。
        实际上，每个可重定位目标文件在 .symtab 中都有一张符号表（除非程序员特意用 STRIP 命令去掉它）。
        然而，和编译器中的符号表不同，.symtab 符号表不包含局部变量的条目。
`.rel.text`：一个 .text 节中位置的列表，当链接器把这个目标文件和其他文件组合时，需要修改这些位置。
            一般而言，任何调用外部函数或者引用全局变量的指令都需要修改。另一方面，调用本地函数的指令则不需要修改。
            注意，可执行目标文件中并不需要重定位信息，因此通常省略，除非用户显式地指示链接器包含这些信息。
`.rel.data`：被模块引用或定义的所有全局变量的重定位信息。
            一般而言，任何已初始化的全局变量，如果它的初始值是一个全局变量地址或者外部定义函数的地址，都需要被修改。
`.debug`：一个调试符号表，其条目是程序中定义的局部变量和类型定义，程序中定义和引用的全局变量，以及原始的 C 源文件。
          只有以 - g 选项调用编译器驱动程序时，才 会得到这张表。
`.line`：原始 C 源程序中的行号和 .text 节中机器指令之间的映射。只有以 -g 选项调用编译器驱动程序时，才会得到这张表。
`.strtab`：一个字符串表，其内容包括 .symtab 和 .debug 节中的符号表，以及节头部中的节名字。字符串表就是以 null 结尾的字符串的序列。


要说明符号表中不同类型符号的概念，我们可以使用一个简单的C语言程序作为例子，并使用像`gcc`和`readelf`这样的工具来编译程序并查看其符号表。

### 示例程序

考虑以下简单的C程序，我们将其命名为`example.c`：

```c
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
```

这个程序包含：
- 一个全局函数 `globalFunction()`，它是一个全局符号。
- 一个静态全局变量 `staticVar`，它是一个局部符号。
- 在`globalFunction`函数内部定义的静态变量 `insideStaticVar`，这也是一个局部符号。

### 编译和查看符号表

首先，使用`gcc`编译这个程序：

```bash
gcc -c example.c
```

这将生成一个名为`example.o`的可重定位目标文件。

然后，使用`readelf`来查看符号表：

```bash
readelf -s example.o
```

这个命令将列出所有在`.symtab`（符号表）节中的符号。

~~~shell
Symbol table '.symtab' contains 21 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS example.c
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    3 
     4: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 
     5: 0000000000000000     4 OBJECT  LOCAL  DEFAULT    3 staticVar
     6: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 
     7: 0000000000000004     4 OBJECT  LOCAL  DEFAULT    3 insideStaticVar.2316
     8: 0000000000000000     0 SECTION LOCAL  DEFAULT    6 
     9: 0000000000000000     0 SECTION LOCAL  DEFAULT    8 
    10: 0000000000000000     0 SECTION LOCAL  DEFAULT    9 
    11: 0000000000000000     0 SECTION LOCAL  DEFAULT   11 
    12: 0000000000000000     0 SECTION LOCAL  DEFAULT   13 
    13: 0000000000000000     0 SECTION LOCAL  DEFAULT   15 
    14: 0000000000000000     0 SECTION LOCAL  DEFAULT   16 
    15: 0000000000000000     0 SECTION LOCAL  DEFAULT   17 
    16: 0000000000000000     0 SECTION LOCAL  DEFAULT   14 
    17: 0000000000000000    23 FUNC    GLOBAL DEFAULT    1 globalFunction
    18: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND _GLOBAL_OFFSET_TABLE_
    19: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND puts
    20: 0000000000000017    25 FUNC    GLOBAL DEFAULT    1 main
~~~

### 解析符号表

在符号表输出中，你将会看到：

- **全局符号**：`globalFunction`会被列出。它由模块定义，并且可以被其他模块引用。

    17: 0000000000000000    23 FUNC    GLOBAL DEFAULT    1 globalFunction

- **局部符号**：`staticVar`和`insideStaticVar`会被列出，因为它们是模块内部定义和引用的静态变量。

     5: 0000000000000000     4 OBJECT  LOCAL  DEFAULT    3 staticVar
     7: 0000000000000004     4 OBJECT  LOCAL  DEFAULT    3 insideStaticVar.2316

- **外部符号**：如果你的程序中有引用其他模块定义的全局变量或函数，它们也会被列为外部符号。在这个例子中，我们没有外部符号，但如果有，它们也会在符号表中显示。

### 注意

- 本地链接器符号与程序中的本地非静态变量不同。例如，函数内的自动变量（局部变量）不会出现在`.symtab`中，因为它们在运行时由栈管理，并非链接器关注的对象。
- 链接器主要关注那些需要在模块之间解析引用的符号，如全局符号和外部符号。局部符号虽然出现在符号表中，但它们仅在定义它们的模块内部可见。

通过实际查看符号表，可以更直观地理解这些不同类型的符号以及它们在编译和链接过程中的作用。

