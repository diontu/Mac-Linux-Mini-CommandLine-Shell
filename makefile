shell: dion-shell.c
	gcc -c dion-shell.c
	gcc -o dion-shell dion-shell.o -lm
