# Loading Executable Object Files
加载可执行目标文件

Q: What are the first a few steps exected after execve() of a ELF dynamic link binary?

When an ELF binary with dynamic linking is executed using `execve()`, several steps are involved to properly set up the process in memory and start its execution. These steps are largely managed by the kernel and the dynamic linker (typically `ld-linux.so` on Linux). Here's an overview of the initial steps that occur:

1. **`execve()` System Call**:
   - The process begins when the `execve()` system call is invoked. This system call tells the kernel to replace the current process image with a new one specified by the ELF binary.

2. **Kernel Loads the ELF Binary**:
   - The kernel first validates the ELF binary, ensuring it's a properly formatted executable.
   - It reads the ELF header to determine the file's structure, including the program header table which describes the segments to be loaded into memory.

3. **Creating a Process Image**:
   - The kernel sets up a new memory space for the process. This involves mapping segments into the process's virtual memory.
   - Segments like `.text` (executable code), `.data` (initialized data), and `.bss` (uninitialized data) are mapped according to their permissions and sizes as specified in the program header table.

4. **Loading the Dynamic Linker**:
   - For dynamically linked binaries, there's an `INTERP` segment which specifies the path to the dynamic linker (e.g., `/lib/ld-linux.so.2`).
   - The kernel loads the dynamic linker into memory as part of the process's image.

5. **Transfer of Control to Dynamic Linker**:
   - Control is passed to the dynamic linker rather than to the entry point of the ELF binary.
   - The dynamic linker initializes itself, which involves setting up various data structures it needs to perform its tasks.

6. **Dynamic Linking**:
   - The dynamic linker processes the dynamic section (`PT_DYNAMIC`) of the ELF binary. This section contains information like symbol and string tables, relocation entries, and dependencies on shared libraries.
   - It loads the required shared libraries into memory.
   - The linker then performs symbol resolution, linking symbols in the program to their definitions in the shared libraries.

7. **Relocations and Initializations**:
   - The linker processes relocations for the ELF binary, adjusting addresses in the binary so that they point to the correct locations.
   - It also calls any initialization functions specified in the shared libraries and the binary itself (e.g., functions registered in `.init_array`).

8. **Starting the Program Execution**:
   - Finally, once all dynamic linking is complete and initializations are done, the dynamic linker jumps to the entry point of the program (as specified in the ELF header), beginning its execution.

Throughout these steps, various other initializations may occur, such as setting up the stack, handling environment variables, and more. This process ensures that the dynamically linked ELF binary is fully prepared with all its dependencies and relocations resolved before its execution begins.

====================================================================================================
当使用 `execve()` 执行一个动态链接的 ELF 二进制文件时，会有几个步骤被执行来正确地在内存中设置进程并开始执行。
这些步骤主要由内核和动态链接器（在 Linux 上通常是 `ld-linux.so`）管理。以下是发生的一些初始步骤概述：

1. **`execve()` 系统调用**：
   - 当调用 `execve()` 系统调用时，进程开始。这个系统调用指示内核用由 ELF 二进制文件指定的新镜像替换当前进程镜像。

2. **内核加载 ELF 二进制文件**：
   - 内核首先验证 ELF 二进制文件，确保它是一个格式正确的可执行文件。
   - 它读取 ELF 头文件来确定文件的结构，包括描述要加载到内存中的段的程序头表。

3. **创建进程镜像**：
   - 内核为进程设置一个新的内存空间。这涉及到将段映射到进程的虚拟内存中。
   - 如 `.text`（可执行代码）、`.data`（已初始化数据）和 `.bss`（未初始化数据）等段根据程序头表中指定的权限和大小被映射。

4. **加载动态链接器**：
   - 对于动态链接的二进制文件，有一个 `INTERP` 段指定了动态链接器的路径（例如 `/lib/ld-linux.so.2`）。
   - 内核将动态链接器作为进程镜像的一部分加载到内存中。

5. **控制权转移给动态链接器**：
   - 控制权被传递给动态链接器，而不是 ELF 二进制文件的入口点。
   - 动态链接器初始化自身，这涉及到设置其执行任务所需的各种数据结构。

6. **动态链接**：
   - 动态链接器处理 ELF 二进制文件的动态段（`PT_DYNAMIC`）。这个段包含了诸如符号和字符串表、重定位条目以及对共享库的依赖等信息。
   - 它将所需的共享库加载到内存中。
   - 链接器然后进行符号解析，将程序中的符号与共享库中的定义链接起来。

7. **重定位和初始化**：
   - 链接器处理 ELF 二进制文件的重定位，调整二进制文件中的地址，使它们指向正确的位置。
   - 它还调用共享库和二进制文件本身指定的任何初始化函数（例如，在 `.init_array` 中注册的函数）。

8. **开始程序执行**：
   - 最后，一旦所有动态链接完成并且初始化完成后，动态链接器跳转到程序的入口点（如 ELF 头文件中所指定），开始执行。

在这些步骤中，还可能发生各种其他初始化，如设置栈、处理环境变量等。
这个过程确保动态链接的 ELF 二进制文件在执行开始前已完全准备好，所有依赖和重定位都已解决。
====================================================================================================


要运行可执行目标文件 prog，我们可以在 Linux shell 的命令行中输入它的名字：

`linux> ./prog`

因为 prog 不是一个内置的 shell 命令，所以 shell 会认为 prog 是一个可执行目标文件，通过调用某个驻留在存储器中称为加载器（loader）的操作系统代码来运行它。
任何 Linux 程序都可以通过调用 `execve` 函数来调用加载器，我们将在 8.4.6 节中详细描述这个函数。
加载器将可执行目标文件中的代码和数据从磁盘复制到内存中，然后通过跳转到程序的第一条指令或入口点来运行该程序。这个将程序复制到内存并运行的过程叫做加载。

每个 Linux 程序都有一个运行时内存映像，类似于图 7-15 中所示。
在 Linux 86-64 系统中，代码段总是从地址 `0x400000` 处开始，后面是数据段。运行时堆(Run-time heap)在数据段之后，通过调用 malloc 库往上增长。
（我们将在 9.9 节中详细描述 malloc 和堆）堆后面的区域是为共享模块保留的。用户栈总是从最大的合法用户地址（2**48 − 1）开始，向较小内存地址增长。
栈上的区域，从地址开始，是为内核（kernel）中的代码和数据保留的，所谓内核就是操作系统驻留在内存的部分。

# Linux x86-64 run-time memory image
# Gaps due to segment alignment requirements and address space layout randomization(ASLR) are not shown. Not to scale

Kernel memory                                                                 
--------------------    2^48 - 1                                           
User stack
(created ar run time)
---------------------   <- `%sep` stack pointer                                                                      
    ⬇

    ⬆
---------------------
Memory-mapped region
for shared libraries

---------------------

    ⬆
---------------------  <- brk
Run-time heap
(created by malloc)

---------------------  
Read/Write segment
(.data, .bss)               \
---------------------        ( Loaded from the executable file)
Read-only code segment      /
(.init, .text, .rodata)
--------------------- 0x400000

--------------------- 0



> 旁注 - 加载器实际是如何工作的？

我们对于加载的描述从概念上来说是正确的，但也不是完全准确，这是有意为之。
要理解加载实际是如何工作的，你必须理解进程、虚拟内存和内存映射的概念，这些我们还没有加以讨论。在后面第 8 章和第 9 章中遇到这些概念时，我们将重新回到加载的问题上，并逐渐向你揭开它的神秘面纱。

对于不够有耐心的读者，下面是关于加载实际是如何工作的一个概述：Linux 系统中的每个程序都运行在一个进程上下文中，有自己的虚拟地址空间。当 shell 运行一个程序时，父 shell 进程生成一个子进程，它是父进程的一个复制。子进程通过 execve 系统调用启动加载器。加载器删除子进程现有的虚拟内存段，并创建一组新的代码、数据、堆和栈段。新的栈和堆段被初始化为零。通过将虚拟地址空间中的页映射到可执行文件的页大小的片（chunk），新的代码和数据段被初始化为可执行文件的内容。最后，加载器跳转到_start 地址，它最终会调用应用程序的 main 函数。除了一些头部信息，在加载过程中没有任何从磁盘到内存的数据复制。直到 CPU 引用一个被映射的虚拟页时才会进行复制，此时，操作系统利用它的页面调度机制自动将页面从磁盘传送到内存。

=========================More Detail================================
这段描述提供了对 ELF 二进制文件在 Linux 系统上加载过程的一个更深入的概述。
它强调了操作系统的进程管理、虚拟内存以及内存映射的概念在这个过程中的关键作用。让我们分步骤详细解释这个过程：

1. **进程上下文与虚拟地址空间**:
   - 在 Linux 系统中，每个程序都在自己的进程上下文中运行，拥有独立的虚拟地址空间。这个地址空间由操作系统管理
   它为程序提供了一个隔离的环境，使程序看似拥有整个系统的资源。

2. **创建子进程**:
   - 当 shell 运行一个程序时，它首先创建一个子进程。这通常通过 `fork()` 系统调用实现，该调用创建了一个父进程的副本。

3. **启动加载器**:
   - 子进程通过 `execve` 系统调用启动加载器。`execve` 替换了当前进程的镜像（包括代码和数据）为新的程序。

4. **删除现有的虚拟内存段**:
   - 加载器清除子进程现有的虚拟内存段，包括之前继承自父进程的代码、数据、堆和栈段。

5. **创建新的段**:
   - 加载器创建一组新的段：代码段、数据段、堆段和栈段。
   - 新的栈和堆段被初始化为空（通常填充为零）。
   - 代码和数据段通过将虚拟地址空间中的页映射到可执行文件的对应页来初始化。这意味着加载器将可执行文件的不同部分映射到进程的虚拟内存中。

6. **懒加载（Lazy Loading）**:
   - 加载过程中，并不立即从磁盘复制所有数据到内存。相反，只有当 CPU 实际引用到某个虚拟页时，操作系统才会通过页面调度机制将该页从磁盘加载到内存中。这种机制被称为“懒加载”或“按需加载”。

7. **跳转到 `_start` 地址**:
   - 一旦所有必要的段被设置好，加载器跳转到程序的 `_start` 地址，这是程序执行的实际起点。`_start` 是一个低级的入口点，它设置了运行环境，然后调用程序的 `main` 函数。

8. **执行 `main` 函数**:
   - 在 `_start` 执行了一些初始化工作之后，最终调用应用程序的 `main` 函数，此时程序开始按照其代码逻辑运行。

这个过程展示了现代操作系统如何高效地管理内存和进程。它使得多个程序可以同时运行，而每个程序都感觉自己独占整个系统资源。同时，懒加载机制提高了内存使用的效率，只有真正需要时才加载数据到内存。