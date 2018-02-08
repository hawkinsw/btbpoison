all: poison pipe poison.obj simple spy victim

spy: spyvictim.c Makefile
	gcc -o spy spyvictim.c -g -O0 -DSPY
victim: spyvictim.c Makefile
	gcc -o victim spyvictim.c -g -O0 -DVICTIM
poison: poison.c Makefile
	gcc -g -o poison poison.c -O0
	gcc -S -g -o poison.s poison.c -O0
pipe: pipe.c Makefile
	gcc -o pipe pipe.c -O0
poison.obj: poison
	objdump -D poison > poison.obj
simple: simple.c Makefile
	gcc -g -o simple simple.c -O0
clean: 
	rm -f poison *.o core pipe simple spy victim
clean_pipes:
	rm -f /tmp/parent_to_child.pipe /tmp/child_to_parent.pipe
