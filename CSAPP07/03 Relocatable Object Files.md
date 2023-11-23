# Relocatable Object Files

图 7-3 展示了一个典型的 ELF 可重定位目标文件的格式。ELF 头（ELF header）以一个 16 字节的序列开始，这个序列描述了生成该文件的系统的字的大小和字节顺序。ELF 头剩下的部分包含帮助链接器语法分析和解释目标文件的信息。其中包括 ELF 头的大小、目标文件的类型（如可重定位、可执行或者共享的）、机器类型（如 X86-64）、节头部表（section header table）的文件偏移，以及节头部表中条目的大小和数量。不同节的位置和大小是由节头部表描述的，其中目标文件中每个节都有一个固定大小的条目（entry）。

+----------------------+
| ELF Header           |
+----------------------+
| Program Header Table |
+----------------------+
| Sections             |
| - .text              |
| - .data              |
| - .bss               |
| - ...                |
+----------------------+
| Section Header Table |
+----------------------+



## Section Header Table:



ELF（Executable and Linkable Format）文件的节头部表（Section Header Table）扮演着重要的角色。
它包含了文件中所有节（sections）的索引和描述信息，每个节可以包含代码、数据、符号表、重定位信息等不同类型的数据。
节头部表使得编译器、链接器和运行时环境能够正确地识别和处理这些节。

### 节头部表的作用

1. **组织和定位节**：
   - 节头部表列出了文件中所有的节及其属性，包括节的名称、大小、地址、类型等。这使得工具和程序能够找到并正确地处理这些节。

2. **链接和装载支持**：
   - 对于链接器来说，节头部表提供了必要的信息以确定如何合并不同的目标文件（.o文件）中的节。对于装载器（loader）来说，它用这些信息来确定如何将节从文件加载到内存中。

3. **调试和分析**：
   - 节头部表对于调试器和分析工具非常重要，它们利用节头部表中的信息来定位代码、数据和其他重要的程序元素。

4. **包含特殊信息**：
   - 某些节可能包含特殊信息，如符号表（提供符号名称、类型、位置等信息）、重定位信息（用于动态链接）、调试信息等。节头部表描述了这些节的属性和位置。

5. **管理权限和属性**：
   - 节头部表中的信息可以指定不同节的内存权限（如可读、可写、可执行）以及是否需要在运行时加载到内存中。

### 示例

在一个典型的ELF文件中，你可能会发现如下类型的节：

- **.text**：包含程序的可执行代码。
- **.data**：包含初始化的全局变量和静态变量。
- **.bss**：包含未初始化的全局变量和静态变量。
- **.symtab**：符号表，包含程序中的函数和变量符号。
- **.strtab**：字符串表，通常用于存储符号名和其他字符串。
- **.rel.text**：包含.text节的重定位信息。
- **.debug**：包含调试信息。

节头部表通过描述这些节的位置、大小和属性，使得操作系统和程序能够正确地处理它们。
尽管程序在运行时主要依赖于程序头表（Program Header Table）来加载程序到内存，但节头部表在链接和调试过程中发挥着至关重要的作用。


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

> 旁注 - 为什么未初始化的数据称为 .bss
> 用术语 .bss 来表示未初始化的数据是很普遍的。它起始于 IBM 704 汇编语言（大约在 1957 年）中“块存储开始（Block Storage Start）”指令的首字母缩写，并沿用至今。一种记住 .data 和 .bss 节之间区别的简单方法是把 “bss” 看成是“更好地节省空间（Better Save Space）” 的缩写




`readelf` 是一个用于显示 ELF（Executable and Linkable Format）文件信息的工具，这种格式是在类 Unix 系统中用于可执行文件、目标代码、共享库和核心转储的标准文件格式。`readelf` 主要用于调试和分析 ELF 文件，并提供了多种选项来查看文件的不同方面。

### 基本用法

- **命令格式**：`readelf [选项] [文件]`
- **功能**：分析和显示 ELF 文件的详细信息。

### 常用选项

1. **`-h`** 或 **`--file-header`**：显示 ELF 文件的文件头信息。
2. **`-l`** 或 **`--program-headers`**：显示程序头信息（也称为段头）。
3. **`-S`** 或 **`--section-headers`**：显示节头信息。
4. **`-s`** 或 **`--syms`**：显示符号表项。
5. **`-d`** 或 **`--dynamic`**：显示动态段信息。
6. **`-r`** 或 **`--relocs`**：显示重定位信息。
7. **`-n`** 或 **`--notes`**：显示注释段信息。
8. **`-x <section>`** 或 **`--hex-dump=<section>`**：以十六进制格式显示指定节的内容。
9. **`-V`** 或 **`--version`**：显示 `readelf` 的版本信息。

### 示例

假设有一个名为 `example` 的 ELF 文件：

1. **查看文件头信息**：
   ```bash
   readelf -h example
   ```
   这个命令显示了 `example` 文件的 ELF 文件头信息，包括魔数、文件类型、机器类型、入口点地址等。

ELF 头：
  Magic：   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  类别:                              ELF64
  数据:                              2 补码，小端序 (little endian)
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              DYN (共享目标文件)
  系统架构:                          Advanced Micro Devices X86-64
  版本:                              0x1
  入口点地址：               0x1060
  程序头起点：          64 (bytes into file)
  Start of section headers:          17000 (bytes into file)
  标志：             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         36
  Section header string table index: 35


2. **查看程序头信息**：
   ```bash
   readelf -l example
   ```
   这将显示文件的程序头（段头）信息，包括各个段的类型、偏移、虚拟地址、物理地址等。

Elf 文件类型为 DYN (共享目标文件)
Entry point 0x1060
There are 13 program headers, starting at offset 64

程序头：
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  PHDR           0x0000000000000040 0x0000000000000040 0x0000000000000040
                 0x00000000000002d8 0x00000000000002d8  R      0x8
  INTERP         0x0000000000000318 0x0000000000000318 0x0000000000000318
                 0x000000000000001c 0x000000000000001c  R      0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x00000000000005f8 0x00000000000005f8  R      0x1000
  LOAD           0x0000000000001000 0x0000000000001000 0x0000000000001000
                 0x0000000000000205 0x0000000000000205  R E    0x1000
  LOAD           0x0000000000002000 0x0000000000002000 0x0000000000002000
                 0x0000000000000188 0x0000000000000188  R      0x1000
  LOAD           0x0000000000002db8 0x0000000000003db8 0x0000000000003db8
                 0x0000000000000258 0x0000000000000260  RW     0x1000
  DYNAMIC        0x0000000000002dc8 0x0000000000003dc8 0x0000000000003dc8
                 0x00000000000001f0 0x00000000000001f0  RW     0x8
  NOTE           0x0000000000000338 0x0000000000000338 0x0000000000000338
                 0x0000000000000020 0x0000000000000020  R      0x8
  NOTE           0x0000000000000358 0x0000000000000358 0x0000000000000358
                 0x0000000000000044 0x0000000000000044  R      0x4
  GNU_PROPERTY   0x0000000000000338 0x0000000000000338 0x0000000000000338
                 0x0000000000000020 0x0000000000000020  R      0x8
  GNU_EH_FRAME   0x0000000000002014 0x0000000000002014 0x0000000000002014
                 0x000000000000004c 0x000000000000004c  R      0x4
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     0x10
  GNU_RELRO      0x0000000000002db8 0x0000000000003db8 0x0000000000003db8
                 0x0000000000000248 0x0000000000000248  R      0x1

 Section to Segment mapping:
  段节...
   00     
   01     .interp 
   02     .interp .note.gnu.property .note.gnu.build-id .note.ABI-tag .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt 
   03     .init .plt .plt.got .plt.sec .text .fini 
   04     .rodata .eh_frame_hdr .eh_frame 
   05     .init_array .fini_array .dynamic .got .data .bss 
   06     .dynamic 
   07     .note.gnu.property 
   08     .note.gnu.build-id .note.ABI-tag 
   09     .note.gnu.property 
   10     .eh_frame_hdr 
   11     
   12     .init_array .fini_array .dynamic .got

3. **查看节头信息**：
   ```bash
   readelf -S example
   ```
   显示文件中所有节的信息，包括节名、类型、地址、大小等。

There are 36 section headers, starting at offset 0x4268:

节头：
  [号] 名称              类型             地址              偏移量
       大小              全体大小          旗标   链接   信息   对齐
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .interp           PROGBITS         0000000000000318  00000318
       000000000000001c  0000000000000000   A       0     0     1
  [ 2] .note.gnu.propert NOTE             0000000000000338  00000338
       0000000000000020  0000000000000000   A       0     0     8
  [ 3] .note.gnu.build-i NOTE             0000000000000358  00000358
       0000000000000024  0000000000000000   A       0     0     4
  [ 4] .note.ABI-tag     NOTE             000000000000037c  0000037c
       0000000000000020  0000000000000000   A       0     0     4
  [ 5] .gnu.hash         GNU_HASH         00000000000003a0  000003a0
       0000000000000024  0000000000000000   A       6     0     8
  [ 6] .dynsym           DYNSYM           00000000000003c8  000003c8
       00000000000000a8  0000000000000018   A       7     1     8
  [ 7] .dynstr           STRTAB           0000000000000470  00000470
       0000000000000082  0000000000000000   A       0     0     1
  [ 8] .gnu.version      VERSYM           00000000000004f2  000004f2
       000000000000000e  0000000000000002   A       6     0     2
  [ 9] .gnu.version_r    VERNEED          0000000000000500  00000500
       0000000000000020  0000000000000000   A       7     1     8
  [10] .rela.dyn         RELA             0000000000000520  00000520
       00000000000000c0  0000000000000018   A       6     0     8
  [11] .rela.plt         RELA             00000000000005e0  000005e0
       0000000000000018  0000000000000018  AI       6    24     8
  [12] .init             PROGBITS         0000000000001000  00001000
       000000000000001b  0000000000000000  AX       0     0     4
  [13] .plt              PROGBITS         0000000000001020  00001020
       0000000000000020  0000000000000010  AX       0     0     16
  [14] .plt.got          PROGBITS         0000000000001040  00001040
       0000000000000010  0000000000000010  AX       0     0     16
  [15] .plt.sec          PROGBITS         0000000000001050  00001050
       0000000000000010  0000000000000010  AX       0     0     16
  [16] .text             PROGBITS         0000000000001060  00001060
       0000000000000195  0000000000000000  AX       0     0     16
  [17] .fini             PROGBITS         00000000000011f8  000011f8
       000000000000000d  0000000000000000  AX       0     0     4
  [18] .rodata           PROGBITS         0000000000002000  00002000
       0000000000000012  0000000000000000   A       0     0     4
  [19] .eh_frame_hdr     PROGBITS         0000000000002014  00002014
       000000000000004c  0000000000000000   A       0     0     4
  [20] .eh_frame         PROGBITS         0000000000002060  00002060
       0000000000000128  0000000000000000   A       0     0     8
  [21] .init_array       INIT_ARRAY       0000000000003db8  00002db8
       0000000000000008  0000000000000008  WA       0     0     8
  [22] .fini_array       FINI_ARRAY       0000000000003dc0  00002dc0
       0000000000000008  0000000000000008  WA       0     0     8
  [23] .dynamic          DYNAMIC          0000000000003dc8  00002dc8
       00000000000001f0  0000000000000010  WA       7     0     8
  [24] .got              PROGBITS         0000000000003fb8  00002fb8
       0000000000000048  0000000000000008  WA       0     0     8
  [25] .data             PROGBITS         0000000000004000  00003000
       0000000000000010  0000000000000000  WA       0     0     8
  [26] .bss              NOBITS           0000000000004010  00003010
       0000000000000008  0000000000000000  WA       0     0     1
  [27] .comment          PROGBITS         0000000000000000  00003010
       000000000000002b  0000000000000001  MS       0     0     1
  [28] .debug_aranges    PROGBITS         0000000000000000  0000303b
       0000000000000030  0000000000000000           0     0     1
  [29] .debug_info       PROGBITS         0000000000000000  0000306b
       000000000000031b  0000000000000000           0     0     1
  [30] .debug_abbrev     PROGBITS         0000000000000000  00003386
       00000000000000e4  0000000000000000           0     0     1
  [31] .debug_line       PROGBITS         0000000000000000  0000346a
       000000000000011e  0000000000000000           0     0     1
  [32] .debug_str        PROGBITS         0000000000000000  00003588
       00000000000002c3  0000000000000001  MS       0     0     1
  [33] .symtab           SYMTAB           0000000000000000  00003850
       00000000000006a8  0000000000000018          34    51     8
  [34] .strtab           STRTAB           0000000000000000  00003ef8
       0000000000000212  0000000000000000           0     0     1
  [35] .shstrtab         STRTAB           0000000000000000  0000410a
       000000000000015a  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  l (large), p (processor specific)


4. **查看符号表**：
   ```bash
   readelf -s example
   ```
   显示文件的符号表，包括函数和变量的名字、类型、位置等。

~/new_space/linking_and_loading/CSAPP07/03  ‹master*› $ readelf -s example

Symbol table '.dynsym' contains 7 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
     2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@GLIBC_2.2.5 (2)
     3: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.2.5 (2)
     4: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
     5: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
     6: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@GLIBC_2.2.5 (2)

Symbol table '.symtab' contains 71 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000318     0 SECTION LOCAL  DEFAULT    1 
     2: 0000000000000338     0 SECTION LOCAL  DEFAULT    2 
     3: 0000000000000358     0 SECTION LOCAL  DEFAULT    3 
     4: 000000000000037c     0 SECTION LOCAL  DEFAULT    4 
     5: 00000000000003a0     0 SECTION LOCAL  DEFAULT    5 
     6: 00000000000003c8     0 SECTION LOCAL  DEFAULT    6 
     7: 0000000000000470     0 SECTION LOCAL  DEFAULT    7 
     8: 00000000000004f2     0 SECTION LOCAL  DEFAULT    8 
     9: 0000000000000500     0 SECTION LOCAL  DEFAULT    9 
    10: 0000000000000520     0 SECTION LOCAL  DEFAULT   10 
    11: 00000000000005e0     0 SECTION LOCAL  DEFAULT   11 
    12: 0000000000001000     0 SECTION LOCAL  DEFAULT   12 
    13: 0000000000001020     0 SECTION LOCAL  DEFAULT   13 
    14: 0000000000001040     0 SECTION LOCAL  DEFAULT   14 
    15: 0000000000001050     0 SECTION LOCAL  DEFAULT   15 
    16: 0000000000001060     0 SECTION LOCAL  DEFAULT   16 
    17: 00000000000011f8     0 SECTION LOCAL  DEFAULT   17 
    18: 0000000000002000     0 SECTION LOCAL  DEFAULT   18 
    19: 0000000000002014     0 SECTION LOCAL  DEFAULT   19 
    20: 0000000000002060     0 SECTION LOCAL  DEFAULT   20 
    21: 0000000000003db8     0 SECTION LOCAL  DEFAULT   21 
    22: 0000000000003dc0     0 SECTION LOCAL  DEFAULT   22 
    23: 0000000000003dc8     0 SECTION LOCAL  DEFAULT   23 
    24: 0000000000003fb8     0 SECTION LOCAL  DEFAULT   24 
    25: 0000000000004000     0 SECTION LOCAL  DEFAULT   25 
    26: 0000000000004010     0 SECTION LOCAL  DEFAULT   26 
    27: 0000000000000000     0 SECTION LOCAL  DEFAULT   27 
    28: 0000000000000000     0 SECTION LOCAL  DEFAULT   28 
    29: 0000000000000000     0 SECTION LOCAL  DEFAULT   29 
    30: 0000000000000000     0 SECTION LOCAL  DEFAULT   30 
    31: 0000000000000000     0 SECTION LOCAL  DEFAULT   31 
    32: 0000000000000000     0 SECTION LOCAL  DEFAULT   32 
    33: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    34: 0000000000001090     0 FUNC    LOCAL  DEFAULT   16 deregister_tm_clones
    35: 00000000000010c0     0 FUNC    LOCAL  DEFAULT   16 register_tm_clones
    36: 0000000000001100     0 FUNC    LOCAL  DEFAULT   16 __do_global_dtors_aux
    37: 0000000000004010     1 OBJECT  LOCAL  DEFAULT   26 completed.8061
    38: 0000000000003dc0     0 OBJECT  LOCAL  DEFAULT   22 __do_global_dtors_aux_fin
    39: 0000000000001140     0 FUNC    LOCAL  DEFAULT   16 frame_dummy
    40: 0000000000003db8     0 OBJECT  LOCAL  DEFAULT   21 __frame_dummy_init_array_
    41: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS example.c
    42: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
    43: 0000000000002184     0 OBJECT  LOCAL  DEFAULT   20 __FRAME_END__
    44: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS 
    45: 0000000000003dc0     0 NOTYPE  LOCAL  DEFAULT   21 __init_array_end
    46: 0000000000003dc8     0 OBJECT  LOCAL  DEFAULT   23 _DYNAMIC
    47: 0000000000003db8     0 NOTYPE  LOCAL  DEFAULT   21 __init_array_start
    48: 0000000000002014     0 NOTYPE  LOCAL  DEFAULT   19 __GNU_EH_FRAME_HDR
    49: 0000000000003fb8     0 OBJECT  LOCAL  DEFAULT   24 _GLOBAL_OFFSET_TABLE_
    50: 0000000000001000     0 FUNC    LOCAL  DEFAULT   12 _init
    51: 00000000000011f0     5 FUNC    GLOBAL DEFAULT   16 __libc_csu_fini
    52: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
    53: 0000000000004000     0 NOTYPE  WEAK   DEFAULT   25 data_start
    54: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND puts@@GLIBC_2.2.5
    55: 0000000000004010     0 NOTYPE  GLOBAL DEFAULT   25 _edata
    56: 00000000000011f8     0 FUNC    GLOBAL HIDDEN    17 _fini
    57: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@@GLIBC_
    58: 0000000000004000     0 NOTYPE  GLOBAL DEFAULT   25 __data_start
    59: 0000000000001149    23 FUNC    GLOBAL DEFAULT   16 printMessage
    60: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
    61: 0000000000004008     0 OBJECT  GLOBAL HIDDEN    25 __dso_handle
    62: 0000000000002000     4 OBJECT  GLOBAL DEFAULT   18 _IO_stdin_used
    63: 0000000000001180   101 FUNC    GLOBAL DEFAULT   16 __libc_csu_init
    64: 0000000000004018     0 NOTYPE  GLOBAL DEFAULT   26 _end
    65: 0000000000001060    47 FUNC    GLOBAL DEFAULT   16 _start
    66: 0000000000004010     0 NOTYPE  GLOBAL DEFAULT   26 __bss_start
    67: 0000000000001160    25 FUNC    GLOBAL DEFAULT   16 main
    68: 0000000000004010     0 OBJECT  GLOBAL HIDDEN    25 __TMC_END__
    69: 0000000000000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
    70: 0000000000000000     0 FUNC    WEAK   DEFAULT  UND __cxa_finalize@@GLIBC_2.2


5. **查看动态段信息**：
   ```bash
   readelf -d example
   ```
   对于动态链接的 ELF 文件，这将显示动态链接信息。

~/new_space/linking_and_loading/CSAPP07/03  ‹master*› $ readelf -d example

Dynamic section at offset 0x2dc8 contains 27 entries:
  标记        类型                         名称/值
 0x0000000000000001 (NEEDED)             共享库：[libc.so.6]
 0x000000000000000c (INIT)               0x1000
 0x000000000000000d (FINI)               0x11f8
 0x0000000000000019 (INIT_ARRAY)         0x3db8
 0x000000000000001b (INIT_ARRAYSZ)       8 (bytes)
 0x000000000000001a (FINI_ARRAY)         0x3dc0
 0x000000000000001c (FINI_ARRAYSZ)       8 (bytes)
 0x000000006ffffef5 (GNU_HASH)           0x3a0
 0x0000000000000005 (STRTAB)             0x470
 0x0000000000000006 (SYMTAB)             0x3c8
 0x000000000000000a (STRSZ)              130 (bytes)
 0x000000000000000b (SYMENT)             24 (bytes)
 0x0000000000000015 (DEBUG)              0x0
 0x0000000000000003 (PLTGOT)             0x3fb8
 0x0000000000000002 (PLTRELSZ)           24 (bytes)
 0x0000000000000014 (PLTREL)             RELA
 0x0000000000000017 (JMPREL)             0x5e0
 0x0000000000000007 (RELA)               0x520
 0x0000000000000008 (RELASZ)             192 (bytes)
 0x0000000000000009 (RELAENT)            24 (bytes)
 0x000000000000001e (FLAGS)              BIND_NOW
 0x000000006ffffffb (FLAGS_1)            标志： NOW PIE
 0x000000006ffffffe (VERNEED)            0x500
 0x000000006fffffff (VERNEEDNUM)         1
 0x000000006ffffff0 (VERSYM)             0x4f2
 0x000000006ffffff9 (RELACOUNT)          3
 0x0000000000000000 (NULL)               0x0

6. **以十六进制格式查看特定节的内容**：
   ```bash
   readelf -x .text example
   ```
   显示 `.text` 节（通常包含程序的机器代码）的内容。

~/new_space/linking_and_loading/CSAPP07/03  ‹master*› $ readelf -x .text example

“.text”节的十六进制输出：
  0x00001060 f30f1efa 31ed4989 d15e4889 e24883e4 ....1.I..^H..H..
  0x00001070 f050544c 8d057601 0000488d 0dff0000 .PTL..v...H.....
  0x00001080 00488d3d d8000000 ff15522f 0000f490 .H.=......R/....
  0x00001090 488d3d79 2f000048 8d05722f 00004839 H.=y/..H..r/..H9
  0x000010a0 f8741548 8b052e2f 00004885 c07409ff .t.H.../..H..t..
  0x000010b0 e00f1f80 00000000 c30f1f80 00000000 ................
  0x000010c0 488d3d49 2f000048 8d35422f 00004829 H.=I/..H.5B/..H)
  0x000010d0 fe4889f0 48c1ee3f 48c1f803 4801c648 .H..H..?H...H..H
  0x000010e0 d1fe7414 488b0505 2f000048 85c07408 ..t.H.../..H..t.
  0x000010f0 ffe0660f 1f440000 c30f1f80 00000000 ..f..D..........
  0x00001100 f30f1efa 803d052f 00000075 2b554883 .....=./...u+UH.
  0x00001110 3de22e00 00004889 e5740c48 8b3de62e =.....H..t.H.=..
  0x00001120 0000e819 ffffffe8 64ffffff c605dd2e ........d.......
  0x00001130 0000015d c30f1f00 c30f1f80 00000000 ...]............
  0x00001140 f30f1efa e977ffff fff30f1e fa554889 .....w.......UH.
  0x00001150 e5488d3d ac0e0000 e8f3feff ff905dc3 .H.=..........].
  0x00001160 f30f1efa 554889e5 b8000000 00e8d7ff ....UH..........
  0x00001170 ffffb800 0000005d c30f1f80 00000000 .......]........
  0x00001180 f30f1efa 41574c8d 3d2b2c00 00415649 ....AWL.=+,..AVI
  0x00001190 89d64155 4989f541 544189fc 55488d2d ..AUI..ATA..UH.-
  0x000011a0 1c2c0000 534c29fd 4883ec08 e84ffeff .,..SL).H....O..
  0x000011b0 ff48c1fd 03741f31 db0f1f80 00000000 .H...t.1........
  0x000011c0 4c89f24c 89ee4489 e741ff14 df4883c3 L..L..D..A...H..
  0x000011d0 014839dd 75ea4883 c4085b5d 415c415d .H9.u.H...[]A\A]
  0x000011e0 415e415f c366662e 0f1f8400 00000000 A^A_.ff.........
  0x000011f0 f30f1efa c3                         .....

### 结论

`readelf` 是一个强大的工具，对于理解和分析 ELF 文件非常有用。它能提供关于文件结构、内容和操作的深入细节，对于开发者和系统管理员来说是不可或缺的工具。根据您的具体需求，可以灵活使用 `readelf` 的不同选项来获取信息。




ELF 头：
  Magic：   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  类别:                              ELF64
  数据:                              2 补码，小端序 (little endian)
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI 版本:                          0
  类型:                              DYN (共享目标文件)
  系统架构:                          Advanced Micro Devices X86-64
  版本:                              0x1
  入口点地址：               0x1060
  程序头起点：          64 (bytes into file)
  Start of section headers:          17000 (bytes into file)
  标志：             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         36
  Section header string table index: 35

### ELF文件头解析
1. **Magic**：
   - 这是ELF文件的魔数，用于标识文件为ELF格式。`7f 45 4c 46` 分别对应ASCII中的 `.`、`E`、`L`、`F`，是所有ELF文件的标准开头。
2. **类别 (Class)**：
   - `ELF64` 表示这是一个64位的ELF文件。
3. **数据 (Data)**：
   - `2 补码，小端序 (little endian)` 表明这个文件使用小端序表示多字节数据。
4. **Version**：
   - 表明这个ELF文件的版本，`1 (current)` 表示当前版本。
5. **OS/ABI**：
   - 指定操作系统和ABI（应用程序二进制接口），这里是 `UNIX - System V`。
6. **ABI 版本**：
   - ABI的具体版本，这里是 `0`。
7. **类型 (Type)**：
   - `DYN (共享目标文件)` 指出这是一个动态链接共享对象文件。
8. **系统架构 (Machine)**：
   - 指出该文件是为哪种架构设计的，这里是 `Advanced Micro Devices X86-64`，即AMD的64位架构。
9. **版本 (Version)**：
   - 文件版本，这里的 `0x1` 表示版本 1。
10. **入口点地址 (Entry point address)**：
    - 这是程序执行开始的虚拟内存地址，这里是 `0x1060`。
11. **程序头起点 (Start of program headers)**：
    - 程序头表在文件中的偏移量，这里是 `64` 字节。
12. **节头起点 (Start of section headers)**：
    - 节头表在文件中的偏移量，这里是 `17000` 字节。
13. **标志 (Flags)**：
    - 特定于架构的标志，这里是 `0x0`。
14. **本头大小 (Size of this header)**：
    - ELF文件头的大小，这里是 `64` 字节。
15. **程序头大小 (Size of program headers)**：
    - 每个程序头的大小，这里是 `56` 字节。
16. **程序头数量 (Number of program headers)**：
    - 程序头的数量，这里是 `13`。
17. **节头大小 (Size of section headers)**：
    - 每个节头的大小，这里是 `64` 字节。
18. **节头数量 (Number of section headers)**：
    - 节头的数量，这里是 `36`。
19. **节头字符串表索引 (Section header string table index)**：
    - 节头字符串表的索引，这里是 `35`。

### 结论

这些信息对于理解ELF文件的结构至关重要，特别是在进行底层编程、调试、或是系统级别的开发时。
例如，知道程序头和节头的位置可以帮助您定位到文件中特定的段或节，而了解入口点地址对于理解程序如何启动也非常重要。