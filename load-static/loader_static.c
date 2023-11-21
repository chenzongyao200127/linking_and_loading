#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>

// #ifndef MAP_ANONYMOUS
// #define MAP_ANONYMOUS MAP_ANON
// #endif


#define STK_SZ           (1 << 20)  // Define stack size (1 MiB)
#define MAP_ANONYMOUS    (1 << 20)  // Define stack size (1 MiB)
#define ROUND(x, align)  (((uintptr_t)x) & ~(align - 1)) // Round down to the nearest multiple of 'align'
#define MOD(x, align)    (((uintptr_t)x) & (align - 1))  // Get the remainder of 'x' divided by 'align'
#define push(sp, T, ...) ({ *((T*)sp) = (T)__VA_ARGS__; \
                            sp = (void *)((uintptr_t)(sp) + sizeof(T)); })

// Custom implementation of execve
void execve_(const char *file, char *argv[], char *envp[]) {
  // WARNING: This execve_ does not free process resources.
  // **NOT** all process states are properly initialized.

  // Open the executable file
  int fd = open(file, O_RDONLY);
  assert(fd > 0);

  // Map the ELF header of the executable into memory
  Elf64_Ehdr *h = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(h != MAP_FAILED);
  assert(h->e_type == ET_EXEC && h->e_machine == EM_X86_64); // Check for valid ELF executable for x86_64

  // Iterate over program headers to map segments into memory
  Elf64_Phdr *pht = (Elf64_Phdr *)((char *)h + h->e_phoff);
  for (int i = 0; i < h->e_phnum; i++) {
    Elf64_Phdr *p = &pht[i];
    if (p->p_type == PT_LOAD) {
      // Memory map region
      // Calculate memory region to map (aligned to p->p_align)
      uintptr_t map_beg = ROUND(p->p_vaddr, p->p_align);
      uintptr_t map_end = map_beg + p->p_memsz;
      while (map_end % p->p_align != 0) map_end++;

      // Memory map flags
      // Set memory protection flags based on segment flags
      int prot = 0;
      if (p->p_flags & PF_R) prot |= PROT_READ;
      if (p->p_flags & PF_W) prot |= PROT_WRITE;
      if (p->p_flags & PF_X) prot |= PROT_EXEC;

      // Memory map size
      // Calculate size to map (aligned to p->p_align)
      int map_sz = p->p_filesz + (p->p_vaddr % p->p_align);
      while (map_sz % p->p_align != 0) map_sz++;

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
      intptr_t extra_sz = p->p_memsz - p->p_filesz;
      if (extra_sz > 0) {
        uintptr_t extra_beg = map_beg + map_sz;
        ret = mmap(
          (void *)extra_beg, extra_sz, prot,  // addr, length, protection
          MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,  // flags
          -1, 0  // no file
        );
        assert(ret != MAP_FAILED);
      }
    }
  }
  close(fd);

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

  // Jump to the entry point of the new program
  asm volatile(
    "mov $0, %%rdx;" // required by ABI
    "mov %0, %%rsp;"
    "jmp *%1" : : "a"(sp_exec), "b"(h->e_entry));
}

int main(int argc, char *argv[], char *envp[]) {
  // Check for correct usage
  if (argc < 2) {
    fprintf(stderr, "Usage: %s file [args...]\n", argv[0]);
    exit(1);
  }
  // Call the custom execve_ function
  execve_(argv[1], argv + 1, envp);
}