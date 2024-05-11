#include <stdio.h>    // Include the standard input/output library for printf
#include <fcntl.h>    // Include for file control options such as open()
#include <unistd.h>   // Include for POSIX constants and read, write, close, lseek
#include <sys/mman.h> // Include for memory management functions like mmap()
#include <stdlib.h>   // Include for data types used in system calls

int main(int argc, char *argv[])
{
  // Check if the correct number of arguments are passed (expecting exactly 1 file name)
  if (argc != 2)
  {
    printf("Usage: %s <file>\n", argv[0]); // Inform the user how to use the program
    return 1;                              // Exit with a non-zero value to indicate an error
  }

  // Open the binary file in read-only mode
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0)
  {
    perror("open"); // Print an error message if file opening fails
    return 1;       // Exit with a non-zero value to indicate an error
  }

  // Get the size of the file by seeking to the end
  off_t file_size = lseek(fd, 0, SEEK_END);
  // Seek back to the beginning of the file to read from the start next
  lseek(fd, 0, SEEK_SET);

  // Map the file into memory for execution
  // mmap() creates a new mapping in the virtual address space of the calling process
  void *mem = mmap(NULL, file_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
  if (mem == MAP_FAILED)
  {
    perror("mmap"); // Print an error message if memory mapping fails
    close(fd);      // Close the file descriptor
    return 1;       // Exit with a non-zero value to indicate an error
  }

  // Close the file as it's no longer needed after mapping
  close(fd);

  // Cast the memory address to a function pointer
  // This treats the loaded binary data as executable code
  void (*binary_func)() = (void (*)())mem;
  binary_func(); // Execute the binary code

  // Clean up: Unmap the memory mapping
  munmap(mem, file_size);

  return 0; // Exit with zero to indicate successful execution
}
