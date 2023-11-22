ld.so: ld.S hello.c
	gcc -fPIC -shared -c ld.S
	ld -shared ld.o -o ld.so
	gcc hello.c -Wl,--dynamic-linker=$(PWD)/ld.so