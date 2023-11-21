~~~c
#define STK_SZ           (1 << 20)  // Define stack size (1 MiB)
#define ROUND(x, align)  (((uintptr_t)x) & ~(align - 1)) // Round down to the nearest multiple of 'align'
#define MOD(x, align)    (((uintptr_t)x) & (align - 1))  // Get the remainder of 'x' divided by 'align'
#define push(sp, T, ...) ({ *((T*)sp) = (T)__VA_ARGS__; \
                            sp = (void *)((uintptr_t)(sp) + sizeof(T)); })
~~~

1. `#define STK_SZ (1 << 20)`
   - 这行代码定义了一个名为 `STK_SZ` 的宏，其值为 `1 << 20`。
   - `(1 << 20)` 是一个位移操作，表示将数字 `1` 向左移动 `20` 位。由于二进制的每一位移相当于乘以 2，所以 `1 << 20` 等于 \(2^{20}\)，即 1048576，这是 1 MiB（兆字节）的字节数。
   - `STK_SZ` 宏通常用于定义一个固定大小的堆栈（stack）空间，这里是 1 MiB。

2. `#define ROUND(x, align) (((uintptr_t)x) & ~(align - 1))`
   - 这个宏用于将一个地址 `x` 向下舍入到最接近的 `align` 的倍数。
   - `uintptr_t` 是一个无符号整型，能够存储一个指针。这里将 `x` 转换成 `uintptr_t` 类型，确保操作是在整数上进行。
   - `~(align - 1)` 产生一个所有低位都是 1，直到 `align` 的二进制位数的掩码。这个掩码用于将 `x` 中低于 `align` 的位清零，从而实现向下舍入。

3. `#define MOD(x, align) (((uintptr_t)x) & (align - 1))`
   - 这个宏用来获取 `x` 除以 `align` 后的余数。
   - 同样，先将 `x` 转换为 `uintptr_t` 类型。
   - `(align - 1)` 产生一个从最低位开始到 `align` 的位数的掩码，这个掩码用于保留 `x` 中低位的值，即余数。

4. `#define push(sp, T, ...) ({ *((T*)sp) = (T)__VA_ARGS__; sp = (void *)((uintptr_t)(sp) + sizeof(T)); })`
   - 这个宏用于在堆栈上推送一个值。
   - `sp` 是堆栈指针，`T` 是要推送的数据类型，`__VA_ARGS__` 是要推送的值。
   - 首先，将堆栈指针 `sp` 转换为类型 `T` 的指针，然后将 `__VA_ARGS__` 赋值给这个地址处。
   - 接着，更新堆栈指针 `sp`，将其向上移动 `sizeof(T)` 个字节，即向堆栈中添加了类型 `T` 的数据。


~~~c
  Elf64_Ehdr *h = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(h != MAP_FAILED);
  assert(h->e_type == ET_EXEC && h->e_machine == EM_X86_64); // Check for valid ELF executable for x86_64
~~~

这段代码的作用是将一个可执行文件的 ELF 头部映射到内存中。
1. **`Elf64_Ehdr *h = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);`**
   - `mmap` 函数用于将文件或者设备的一部分内容映射到内存中。这里它被用来映射 ELF 文件的头部。
   - `NULL` 参数指定 `mmap` 可以选择映射区域的起始地址。当为 `NULL` 时，操作系统会自动选择一个地址。
   - `4096` 是映射区域的大小。这里选择 4096 字节（一个标准的内存页大小），因为 ELF 头部通常不会超过这个大小。
   - `PROT_READ` 表示映射区域可读。
   - `MAP_PRIVATE` 表示创建一个写时复制的私有映射。这意味着对该内存区域的更改不会写回到原文件中。
   - `fd` 是之前打开的 ELF 文件的文件描述符。
   - 最后一个参数 `0` 表示映射区域从文件的开头开始。

2. **`assert(h != MAP_FAILED);`**
   - 这行代码检查 `mmap` 调用是否成功。如果 `mmap` 失败，它会返回 `MAP_FAILED`（通常是 `(void *)-1`）。
   这里的 `assert` 用于验证 `mmap` 没有失败。

3. **`assert(h->e_type == ET_EXEC && h->e_machine == EM_X86_64);`**
   - 这行代码检查映射到内存中的 ELF 头部是否为有效的 x86_64 可执行文件。
   - `h->e_type == ET_EXEC` 确认这是一个可执行文件（而不是动态链接库或其他类型的 ELF 文件）。
   - `h->e_machine == EM_X86_64` 确认这个可执行文件是为 x86_64 架构编译的。

整体来说，这部分代码的目的是将 ELF 文件的头部加载到内存中，
以便进一步解析和处理 ELF 文件，如读取程序头表（Program Header Table）来映射程序的其他部分。
这是实现自定义 `execve` 功能的一个关键步骤，它允许程序直接解析和加载 ELF 可执行文件。

~~~c
  // Iterate over program headers to map segments into memory
  // 这行代码计算程序头表的地址。
  // h->e_phoff 是 ELF 头部中指定的程序头表的偏移量，h 是指向 ELF 头部的指针。将这两者相加得到程序头表的实际内存地址。
  Elf64_Phdr *pht = (Elf64_Phdr *)((char *)h + h->e_phoff);

  // 这个循环遍历程序头表中的每一个条目。h->e_phnum 是程序头表中条目的数量。
  for (int i = 0; i < h->e_phnum; i++) {
    // 这个条件判断确定当前的程序头表条目是否指示一个需要被加载到内存中的段（PT_LOAD 类型）。
    Elf64_Phdr *p = &pht[i];
    if (p->p_type == PT_LOAD) {
      // Memory map region
      // Calculate memory region to map (aligned to p->p_align)
      // 计算映射区域
      uintptr_t map_beg = ROUND(p->p_vaddr, p->p_align); // 计算要映射的段的开始地址，按照 p->p_align 对齐。
      uintptr_t map_end = map_beg + p->p_memsz; //  计算段的结束地址。
      while (map_end % p->p_align != 0) map_end++; // 确保结束地址也按照对齐要求调整。

      // Memory map flags
      // Set memory protection flags based on segment flags
      // 据段的标志（p->p_flags），设置内存映射的保护级别（读、写、执行）。
      int prot = 0;
      if (p->p_flags & PF_R) prot |= PROT_READ;
      if (p->p_flags & PF_W) prot |= PROT_WRITE;
      if (p->p_flags & PF_X) prot |= PROT_EXEC;

      // Memory map size
      // Calculate size to map (aligned to p->p_align)
      int map_sz = p->p_filesz + (p->p_vaddr % p->p_align); //  计算要映射的大小，确保包括文件大小和对齐所需的额外空间。
      while (map_sz % p->p_align != 0) map_sz++; //  确保映射大小符合对齐要求。

      // Map file contents to memory
      void *ret = mmap(
        (void *)map_beg,                 // addr, rounded to ALIGN
        map_sz,                          // length
        prot,                            // protection
        MAP_PRIVATE | MAP_FIXED,         // flags, private & strict
        fd,                              // file descriptor
        ROUND(p->p_offset, p->p_align)   // offset
      );
      assert(ret != MAP_FAILED);

      // Map additional anonymous memory if needed (e.g., for .bss section)
      // 映射额外匿名内存
      // 对于那些内存大小（p->p_memsz）比文件大小（p->p_filesz）大的段（如 .bss 段）
      // 需要额外映射匿名内存来存储初始化为零的数据。
      intptr_t extra_sz = p->p_memsz - p->p_filesz;
      if (extra_sz > 0) {
        uintptr_t extra_beg = map_beg + map_sz;
        // 再次使用 mmap，但这次使用 MAP_ANONYMOUS 标志来映射额外所需的内存。
        ret = mmap(
          (void *)extra_beg, extra_sz, prot,  // addr, length, protection
          MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,  // flags
          -1, 0  // no file
        );
        assert(ret != MAP_FAILED);
      }
    }
  }

  // 完成所有映射操作后，关闭之前打开的文件描述符 fd。
  close(fd);
~~~


这段代码中的对齐操作非常重要，因为它们确保了程序段（segments）被正确地映射到进程的地址空间中。
在深入解释之前，我们需要理解几个关键点：

1. **内存对齐的重要性**：
   - 在计算机系统中，内存对齐是确保数据按照特定边界对齐的过程。对齐通常是为了提高内存访问的效率，
   因为许多硬件平台对非对齐的内存访问有性能惩罚，甚至可能不支持。
   - 在映射程序段时，段的地址和大小通常需要按照一定的边界（如页边界）对齐，以符合操作系统和硬件的要求。

2. **ELF 文件结构**：
   - ELF（Executable and Linkable Format）文件包含了程序运行所需的所有信息，包括代码、数据、程序入口点等。
   这些信息以“段”（如文本段、数据段）的形式存储。
   - ELF 文件的头部（ELF header）描述了整个文件的结构，而程序头表（Program Header Table）则描述了各个段在内存中的映射方式。

那么，这段代码具体在做什么？

1. **映射 ELF 文件的各个段到内存**：
   - 代码首先读取 ELF 文件的头部和程序头表，来确定哪些段需要被加载到内存中。
   - 对于每个需要加载的段（标记为 `PT_LOAD`），代码计算出这个段在内存中的起始地址（`map_beg`）和结束地址（`map_end`），以及需要映射的大小（`map_sz`）。

2. **确保对齐**：
   - 段的地址和大小必须按照一定的边界进行对齐，这通常是内存页的大小（例如 4KB）。这样做是为了保证操作系统页映射机制的正确性和效率。
   - `ROUND` 宏用于将地址向下舍入到最近的对齐边界，以确保段的起始地址是正确对齐的。
   - 代码中的循环（`while (map_end % p->p_align != 0) map_end++;`）确保结束地址也是对齐的。

3. **处理 `.bss` 段等额外内存需求**：
   - 对于 `.bss` 段（未初始化的全局变量存储区），其在文件中不占用空间（`p->p_filesz` 为零），但在内存中需要一定的空间并且内容初始化为零。这就需要映射额外的、初始化为零的匿名内存。

通过这些操作，代码在进程的地址空间中手动映射了 ELF 文件的所有必要段，准备执行程序。
这是一个相对低级且复杂的过程，通常由操作系统的加载器自动完成，但在某些特殊场景下（如自制的简易操作系统或特殊的执行环境），可能需要手动进行这些操作。

~~~c
  // Set up the stack for the new program
  static char stack[STK_SZ], rnd[16];
  void *sp = (void *)ROUND(stack + sizeof(stack) - 4096, 16);
  void *sp_exec = sp;
  int argc = 0;

  // argc
  while (argv[argc]) argc++;
  push(sp, intptr_t, argc);

  // argv[], NULL-terminate
  for (int i = 0; i <= argc; i++)
    push(sp, intptr_t, argv[i]);

  // envp[], NULL-terminate
  for (; *envp; envp++) {
    if (!strchr(*envp, '_')) // remove some verbose ones
      push(sp, intptr_t, *envp);
  }
  
  // auxv[], AT_NULL-terminate
  // Push auxiliary vector onto the stack
  push(sp, intptr_t, 0);
  push(sp, Elf64_auxv_t, { .a_type = AT_RANDOM, .a_un.a_val = (uintptr_t)rnd } );
  push(sp, Elf64_auxv_t, { .a_type = AT_NULL } );
~~~

这段代码的目的是为新程序设置堆栈，并准备程序执行时需要的参数和环境变量。下面是对代码的详细解释：

1. **设置新程序的堆栈**：
   - `static char stack[STK_SZ], rnd[16];` 
   声明了一个固定大小的数组 `stack` 用作新程序的堆栈，其大小由 `STK_SZ` 定义（1 MiB）。
   `rnd` 数组用于存储一些随机数据，可能被用作辅助向量（auxiliary vector）中的随机种子。

   - `void *sp = (void *)ROUND(stack + sizeof(stack) - 4096, 16);` 初始化堆栈指针 `sp`。
   这里从 `stack` 数组的末尾减去 4096 字节开始（为了留出一些空间），然后通过 `ROUND` 宏确保堆栈指针是按 16 字节对齐的。

2. **准备 `argc`、`argv[]` 和 `envp[]`**：
   - 这些循环和 `push` 操作用于将命令行参数（`argc` 和 `argv[]`）以及环境变量（`envp[]`）推送到堆栈上。
   - 首先计算 `argc`（参数数量），然后将每个参数和环境变量的地址推送到堆栈上。
   这些数据需要按照特定的格式排列，以便新程序可以正确地解析它们。

3. **处理辅助向量（auxiliary vector）**：
   - 辅助向量用于传递一些系统级信息给新程序，例如系统页的大小、随机种子等。
   - 代码首先推送一个值为 0 的 `intptr_t` 类型的数据，表示辅助向量的结束。
   - 然后推送一个 `Elf64_auxv_t` 类型的辅助向量条目，其中包含类型 `AT_RANDOM` 和一个指向 `rnd` 数组的指针，这可能用于提供一些随机数据。
   - 最后推送另一个 `Elf64_auxv_t` 类型的条目，类型为 `AT_NULL`，表示辅助向量的结束。

通过这些步骤，代码为新程序准备了一个格式正确的堆栈，包含了程序的所有参数、环境变量以及必要的系统信息。这样做是为了模仿操作系统在创建新进程时的行为，确保新程序能够正确启动并访问到它需要的所有信息。


~~~c
  // argc
  while (argv[argc]) argc++;
  push(sp, intptr_t, argc);

  // argv[], NULL-terminate
  for (int i = 0; i <= argc; i++)
    push(sp, intptr_t, argv[i]);

  // envp[], NULL-terminate
  for (; *envp; envp++) {
    if (!strchr(*envp, '_')) // remove some verbose ones
      push(sp, intptr_t, *envp);
  }
~~~

这部分代码负责将程序的参数计数（`argc`）、参数列表（`argv[]`）、和环境变量列表（`envp[]`）推送到新程序的堆栈上。
这是为了准备新程序运行时所需的传统的命令行接口。下面是对每部分的详细解释：

1. **处理 `argc`（参数计数）**：
   - `while (argv[argc]) argc++;` 这个循环计算 `argv` 数组中的元素个数，即命令行参数的数量。
   它通过遍历直到遇到 `NULL` 指针来实现。
   - `push(sp, intptr_t, argc);` 然后使用 `push` 宏将 `argc`（参数的数量）推送到堆栈上。`push` 宏首先将值存储在由堆栈指针 `sp` 指向的地址上，然后将堆栈指针向上移动以为下一个数据留出空间。

2. **处理 `argv[]`（参数列表）**：
   - `for (int i = 0; i <= argc; i++) push(sp, intptr_t, argv[i]);` 这个循环将 `argv` 数组中的每个元素（即每个参数的指针）推送到堆栈上。
   循环条件 `i <= argc` 确保了 `argv[argc]`（即 `NULL` 指针，标记参数列表的结束）也被推送到堆栈上，这是 UNIX 系统程序所期望的格式。

3. **处理 `envp[]`（环境变量列表）**：
   - `for (; *envp; envp++) { ... }` 这个循环遍历 `envp` 数组，即环境变量的列表。
   - `if (!strchr(*envp, '_')) push(sp, intptr_t, *envp);` 在这里，代码选择性地将某些环境变量推送到堆栈上。
   它通过检查每个环境变量字符串是否包含下划线字符来决定是否推送。这可能是为了去除某些过于冗长或不必要的环境变量。

通过这些步骤，新程序的堆栈上依次排列了 `argc`、`argv[]`（以 `NULL` 结尾）和 `envp[]`（也以 `NULL` 结尾）。
这样，当新程序开始执行时，它就可以通过标准的方式访问其命令行参数和环境变量。这是 UNIX 和类 UNIX 系统中程序启动时的常见模式。


~~~c
  // auxv[], AT_NULL-terminate
  // Push auxiliary vector onto the stack
  push(sp, intptr_t, 0);
  push(sp, Elf64_auxv_t, { .a_type = AT_RANDOM, .a_un.a_val = (uintptr_t)rnd } );
  push(sp, Elf64_auxv_t, { .a_type = AT_NULL } );
~~~

这部分代码处理的是将辅助向量（auxiliary vector）推送到新程序的堆栈上。
辅助向量是在程序启动时传递给程序的一组键值对，它们提供了有关程序执行环境的各种信息。下面是对每个操作的详细解释：

1. **结束辅助向量的标志**：
   - `push(sp, intptr_t, 0);` 
   这行代码将一个值为 0 的整数推送到堆栈上。
   在辅助向量的上下文中，一个零值用来标识辅助向量的结束。
   这是一个传统的UNIX约定，用于标识辅助向量数组的终结。

2. **推送随机数种子到辅助向量**：
   - `push(sp, Elf64_auxv_t, { .a_type = AT_RANDOM, .a_un.a_val = (uintptr_t)rnd });` 
   这行代码创建了一个辅助向量条目，并将其推送到堆栈上。辅助向量条目由类型（`a_type`）和值（`a_un.a_val`）组成。
   - `.a_type = AT_RANDOM` 指明这个条目包含随机数种子。
   - `.a_un.a_val = (uintptr_t)rnd` 设置这个条目的值为 `rnd` 数组的地址，这里假设 `rnd` 数组存储了一些随机数据。

3. **结束辅助向量**：
   - `push(sp, Elf64_auxv_t, { .a_type = AT_NULL });` 
   这行代码推送了另一个辅助向量条目，类型为 `AT_NULL`。
   这个特殊的类型用于标记辅助向量数组的末尾，表示没有更多的辅助向量条目。

通过这些步骤，代码成功地将一个简化版的辅助向量推送到了新程序的堆栈上。
辅助向量提供了运行时环境相关的重要信息，如随机数种子、系统页大小、硬件功能等，有助于程序正确配置其运行环境。
在标准的UNIX系统中，这一过程通常由操作系统在程序启动时自动完成。
在自定义的程序加载器中实现这一步骤，有助于确保新程序能够在一个标准的执行环境中运行。


~~~c
  // Jump to the entry point of the new program
  asm volatile(
    "mov $0, %%rdx;" // required by ABI
    "mov %0, %%rsp;"
    "jmp *%1" : : "a"(sp_exec), "b"(h->e_entry));
~~~

这部分代码负责将执行流转移到新程序的入口点。
它使用内联汇编来直接操作处理器寄存器，以便正确设置执行新程序所需的状态。
下面是对这段代码的详细解释：

1. **`asm volatile(...);`**:
   - 这是 GCC 的内联汇编语法，用于在 C 代码中直接嵌入汇编指令。
   `volatile` 关键字告诉编译器不要优化这段汇编代码，确保按照程序员的意图精确执行。

2. **`"mov $0, %%rdx;"`**:
   - 这条汇编指令将数字 `0` 移动到 `rdx` 寄存器。
   在 Linux x86_64 的应用程序二进制接口（ABI）中，
   `rdx` 通常用于传递系统调用的第三个参数。这里将其置为 0，可能是为了遵循某些约定或清除之前的状态。

3. **`"mov %0, %%rsp;"`**:
   - 这条指令将 `sp_exec`（之前设置好的新程序堆栈指针）移动到 `rsp` 寄存器，即堆栈指针寄存器。
   在 x86_64 架构中，`rsp` 用于指向当前堆栈的顶部。
   这一步是在准备新程序的堆栈，确保当新程序开始执行时，它有正确设置的堆栈。

4. **`"jmp *%1"`**:
   - 这条指令是一个间接跳转，它使用 `h->e_entry`（新程序的入口地址）作为跳转目标。
   这实际上将控制权转移到新程序，开始其执行。

这段代码的作用是将当前执行上下文转换到新程序：它设置了新的堆栈指针并跳转到新程序的入口点。这是手动实现进程替换的关键步骤，类似于操作系统在执行 `execve` 系统调用时所做的操作。
此操作非常底层且具有一定风险，因为它涉及直接操作处理器状态和内存，要求程序员对操作系统和硬件架构有深入了解。
