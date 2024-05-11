# What are the first a few steps exected after `execve()` of a ELF dynamic link binary?

When an ELF (Executable and Linkable Format) dynamic link binary is executed using the `execve()` system call in a Linux environment, several steps occur in a specific sequence to set up the process for execution. 

Here's a detailed breakdown of the initial steps after `execve()` is invoked:

1. **Loading the ELF Binary:**
   - The kernel validates the ELF header of the binary to ensure it's a valid executable. This includes checking the magic number, the file type, and the machine type.
   - The kernel reads the program headers from the ELF binary. These headers describe how the binary should be loaded into memory.
   1. 内核验证二进制文件的 ELF 头部，确保它是有效的可执行文件。这包括检查魔数（magic number）、文件类型和机器类型。
   2. 内核从 ELF 二进制文件中读取程序头部。这些头部描述了如何将二进制文件加载到内存中。

2. **Setting up the Process Address Space:**
   - The kernel sets up a new address space for the process.
   - Memory regions are allocated for the binary's code (text segment), data (data segment), BSS (uninitialized data), and stack.
   - The binary is mapped into the process's address space. This involves copying the code and initialized data from the binary into the respective memory regions.
   1. 内核为进程设置新的地址空间。
   2. 为二进制文件的代码（文本段）、数据（数据段）、BSS（未初始化数据）和栈分配内存区域。
   3. 将二进制文件映射到进程的地址空间中。这涉及将代码和初始化数据从二进制文件复制到相应的内存区域。

3. **Dynamic Linking:**
   - If the binary is dynamically linked (as specified in your question), the dynamic linker (`ld-linux.so.2` or `ld-linux-x86-64.so.2` for x86 and x86_64 architectures, respectively) is loaded into the process's address space.
   - The dynamic linker then parses the dynamic section of the ELF binary to identify the shared libraries that the binary depends on.
   - The necessary shared libraries are mapped into the process's address space. The dynamic linker handles any symbol resolution and relocation that needs to occur for these libraries.

   1. 如果二进制文件是动态链接的（如您的问题所述），则将动态链接器（例如 x86 和 x86_64 架构的 ld-linux.so.2 或 ld-linux-x86-64.so.2）加载到进程的地址空间中。
   2. 动态链接器接着解析 ELF 二进制文件的动态部分，以识别二进制文件依赖的共享库。
   3. 必要的共享库被映射到进程的地址空间中。动态链接器处理这些库的任何符号解析和重定位。

4. **Initialization:**
   - The dynamic linker calls the initialization routines of the loaded shared libraries (`.init` section).
   - The dynamic linker also handles TLS (Thread-Local Storage) initialization if required by the binary or any of the shared libraries.
  
   1. 动态链接器调用加载的共享库的初始化例程（.init 部分）。
   2. 如果二进制文件或任何共享库需要，动态链接器还处理 TLS（线程本地存储）初始化。

5. **Transfer of Control:**
   - After all the initialization is complete, the dynamic linker transfers control to the program's entry point (usually the `main` function).
   - At this point, the program begins its normal execution.

   1. 在所有初始化完成后，动态链接器将控制权转移到程序的入口点（通常是 main 函数）。
   2. 此时，程序开始其正常执行。

6. **Stack Preparation:**
   - Just before transferring control to the program's entry point, the kernel sets up the initial stack. This stack contains the program arguments (`argv`), environment variables (`envp`), auxiliary vectors (providing information about the runtime environment), and the initial stack pointer.

   1. 在将控制权转移到程序的入口点之前，内核设置初始栈。
   2. 这个栈包含程序参数（argv）、环境变量（envp）、辅助向量（提供有关运行时环境的信息）和初始栈指针。

7. **Execution of `main`:**
   - The program now starts executing from its `main` function with the stack properly set up to include arguments and environment variables.

   1. 程序现在从其 main 函数开始执行，栈已正确设置，包括参数和环境变量。

These steps are a high-level overview and may involve more nuanced operations depending on the specific architecture, kernel version, and the binary itself. 

The entire process is designed to be efficient and secure, ensuring that the binary is correctly loaded and executed in a new process context.

==============================================================================================================

The detailed concepts involved in the dynamic linking process, specifically when the dynamic linker parses the dynamic section of the ELF binary and maps the necessary shared libraries into the process's address space, involve several key components and steps:

当动态链接器解析 ELF 二进制文件的动态部分并将必要的共享库映射到进程的地址空间时，动态链接过程中涉及的详细概念包括几个关键组成部分和步骤：

1. **Parsing the Dynamic Section:**
   - Each ELF binary has a dynamic section (`.dynamic`) if it is dynamically linked. This section contains an array of entries, each specifying a particular type of data necessary for dynamic linking.
   - These entries include the names and locations of shared libraries (`.so` files) that the binary depends on, symbol tables, relocation tables, and other runtime requirements.

   如果 ELF 二进制文件是动态链接的，它将具有一个动态部分（`.dynamic`）。这个部分包含了一个条目数组，每个条目指定了动态链接所需的特定类型的数据。
   这些条目包括共享库（`.so` 文件）的名称和位置，符号表，重定位表，以及其他运行时要求。

2. **Identifying Shared Libraries:**
   - Among the entries in the dynamic section, there are specific tags (such as `DT_NEEDED`) that list the names of shared libraries required by the binary.
   - The dynamic linker reads these entries to determine which shared libraries must be loaded.

   在动态部分的条目中，有特定的标签（如 `DT_NEEDED`），列出了二进制文件所需的共享库的名称。
   动态链接器读取这些条目，以确定哪些共享库必须被加载。

3. **Loading Shared Libraries:**
   - The dynamic linker loads the required shared libraries into memory. This is typically done by mapping the libraries from disk into the process's address space.
   - Each shared library is itself an ELF file with its own set of program headers, dynamic section, text segment, data segment, etc.

   动态链接器将所需的共享库加载到内存中。通常，这是通过将库从磁盘映射到进程的地址空间来完成的。
   每个共享库本身都是一个具有自己一套程序头部、动态部分、文本段、数据段等的 ELF 文件。

4. **Symbol Resolution:**
   - Programs use symbols (such as function or variable names) to refer to code or data. These symbols need to be resolved to actual memory addresses.
   - The dynamic linker searches the symbol tables of the shared libraries and the executable to match each symbol used in the program with its corresponding address.
   - This process is known as symbol resolution. If a symbol is defined in multiple places, rules such as symbol interposition are used to determine which definition to use.

   程序使用符号（如函数或变量名）来引用代码或数据。这些符号需要被解析为实际的内存地址。
   动态链接器在共享库和可执行文件的符号表中搜索，将程序中使用的每个符号与其对应的地址匹配。
   这个过程称为符号解析。如果一个符号在多个地方定义，将使用如符号插入等规则来确定使用哪个定义。

5. **Relocation:**
   - Relocation involves adjusting the symbol addresses in the program so they point to the correct locations.
   - Entries in the relocation tables of the ELF binary indicate which parts of the code need to be updated with the actual addresses of the symbols.
   - The dynamic linker adjusts these references, which may involve modifying code and data segments.

   重定位涉及调整程序中的符号地址，使其指向正确的位置。
   ELF 二进制文件的重定位表中的条目指出哪些代码部分需要用符号的实际地址进行更新。
   动态链接器调整这些引用，可能涉及修改代码和数据段。

6. **Handling Dependencies of Shared Libraries:**
   - Shared libraries can themselves depend on other shared libraries. The dynamic linker recursively processes each library's dynamic section to resolve these dependencies.

   共享库本身可能依赖于其他共享库。动态链接器递归处理每个库的动态部分，以解决这些依赖关系。

7. **Initialization and Finalization:**
   - Shared libraries may have initialization (`_init`) and finalization (`_fini`) functions. The dynamic linker calls these functions before and after the program's execution, respectively.
   
   共享库可能有初始化（`_init`）和终结（`_fini`）函数。动态链接器在程序执行前后分别调用这些函数。

In summary, dynamic linking is a sophisticated process that involves parsing the dynamic section of an ELF binary, loading the necessary shared libraries, resolving symbols to their actual addresses in memory, and performing relocations. This process allows programs to use shared code from libraries efficiently, reducing memory usage and disk space requirements.

总之，动态链接是一个复杂的过程，涉及解析 ELF 二进制文件的动态部分，加载必要的共享库，将符号解析为内存中的实际地址，并执行重定位。这个过程允许程序高效地使用来自库的共享代码，减少了内存使用和磁盘空间需求。

==============================================================================================================

## TLS

TLS (Thread-Local Storage) is a programming construct that allows static or global variables to be unique to each thread in a multi-threaded process. It is used to store data that is unique to a specific thread, ensuring that each thread has its own separate instance of certain variables.

线程局部存储（Thread-Local Storage，简称 TLS）是一种编程构造，它允许在多线程进程中为每个线程提供静态或全局变量的独立副本。
TLS 用于存储特定于单个线程的数据，确保每个线程都拥有某些变量的自己的独立实例。

Here's a more detailed explanation:

1. **Basic Concept:**
   - In a single-threaded application, global and static variables are shared across the entire process. However, in a multi-threaded application, you might want certain variables to be isolated to each thread, so they are not shared or affected by other threads.
   - TLS provides a mechanism to create such thread-specific data, where each thread gets its own separate copy of the variable.

   在单线程应用程序中，全局和静态变量在整个进程中共享。然而，在多线程应用程序中，可能需要某些变量被隔离到每个线程中，以便它们不被其他线程共享或影响。
   TLS 提供了一种机制来创建这样的线程特定数据，其中每个线程都获得变量的自己的单独副本。

2. **Usage and Benefits:**
   - TLS is particularly useful in situations where different threads need to maintain their own state or context. For example, this can be useful for storing per-thread caches, random number generators, or other resources that are expensive or non-trivial to share across threads.
   - Using TLS can help avoid issues with concurrency and synchronization, as the data in TLS is inherently thread-safe (since it is accessed only by one thread).

   在不同线程需要维护自己的状态或上下文的情况下，TLS 特别有用。例如，这对于存储每个线程的缓存、随机数生成器或其他难以跨线程共享的资源非常有用。
   使用 TLS 可以帮助避免并发和同步问题，因为 TLS 中的数据本质上是线程安全的（因为它只被一个线程访问）。   

3. **Implementation:**
   - In most programming environments, TLS can be implemented using specific keywords or compiler directives. For example, in C and C++, the `thread_local` keyword is used to declare thread-local variables.
   - In Java, thread-local storage is provided through the `ThreadLocal` class, which manages the storage of thread-specific values.

   在大多数编程环境中，可以使用特定的关键字或编译器指令来实现 TLS。例如，在 C 和 C++ 中，使用 thread_local 关键字来声明线程局部变量。
   在 Java 中，通过 ThreadLocal 类提供线程局部存储，该类管理线程特定值的存储。

4. **Operating System Support:**
   - TLS is often supported by the operating system and the runtime environment of the programming language. For example, in Linux, the ELF format (used for executables and shared libraries) has specific provisions for defining and handling TLS.

   TLS 通常由操作系统和编程语言的运行时环境支持。例如，在 Linux 中，用于可执行文件和共享库的 ELF 格式对定义和处理 TLS 有特定的规定。

5. **Limitations:**
   - While TLS is useful for isolating data to individual threads, it can increase the complexity of the program, especially in the management of the lifecycle of the thread-local data.
   - Overuse of TLS can lead to increased memory usage, as each thread will have its own copy of the data.

   虽然 TLS 对于隔离线程数据很有用，但它可能会增加程序的复杂性，特别是在管理线程局部数据的生命周期方面。
   过度使用 TLS 可能导致内存使用量增加，因为每个线程都将拥有数据的自己的副本。

In summary, TLS is a powerful tool for managing thread-specific data in multi-threaded applications, allowing for cleaner and more efficient handling of data that needs to be unique to each thread. It provides a way to avoid issues with shared data access in concurrent programming, though it requires careful consideration in terms of memory usage and program complexity.

总之，TLS 是一种强大的工具，用于在多线程应用程序中管理线程特定数据，允许更清晰、更高效地处理需要对每个线程唯一的数据。
它提供了一种避免并发编程中共享数据访问问题的方式，尽管它需要在内存使用和程序复杂性方面进行仔细考虑。