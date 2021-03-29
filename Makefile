allocate: allocate.c
	gcc -o allocate allocate.c cpu.c events.c process.c -lm
clean: 
	rm -f allocate
