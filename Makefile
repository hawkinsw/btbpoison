all: poison pipe poison.obj

poison: poison.c Makefile
	gcc -g -o poison poison.c -O0
pipe: pipe.c Makefile
	gcc -o pipe pipe.c -O0
poison.obj: poison
	objdump -D poison > poison.obj
clean: 
	rm -f poison *.o core poison.obj pipe
