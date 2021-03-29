allocate:
	gcc -o allocate allocate.c cpu.c events.c process.c processesWaiting.c -lm
clean: 
	rm -f allocate
