# This is the rule for building the 'loader' target.
# It compiles 'loader.c' using gcc with debug symbols (-ggdb) and outputs an executable named 'loader'.
loader: loader.c
	gcc -ggdb $< -o $@

# This is the rule for building the 'minimal' target, which is an assembly program.
# It first compiles 'minimal.S' into an assembly file 'minimal.s'.
# Then it assembles 'minimal.s' into an object file 'minimal.o'.
# Afterwards, it links 'minimal.o' to create an executable 'minimal'.
# Finally, it uses objcopy to extract the .text section from the 'minimal' executable and creates a binary file 'minimal.bin'.
minimal: minimal.S
	gcc -S minimal.S > minimal.s  # Compiles the assembly source file to a .s file
	as minimal.s -o minimal.o     # Assembles the .s file into an object file
	ld minimal.o -o $@            # Links the object file to create an executable
	objcopy -O binary --only-section=.text minimal minimal.bin  # Extracts the .text section into a binary file

# This is the clean rule used to remove all generated files.
# It removes the 'loader' and 'minimal' executables, object files (*.o), assembly files (*.s), and binary files (*.bin).
clean:
	rm -f loader minimal *.o *.s *.bin
