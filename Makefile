all: poison pipe poison.obj time_mispredict_asm spy victim time_mispredict_c

spy: spyvictim.c Makefile
	gcc -o spy spyvictim.c -g -O0 -DSPY -std=gnu99
victim: spyvictim.c Makefile
	gcc -o victim spyvictim.c -g -O0 -DVICTIM -std=gnu99
poison: poison.c Makefile
	gcc -g -o poison poison.c -O0 -std=gnu99
	gcc -S -g -o poison.s poison.c -O0 -std=gnu99
pipe: pipe.c Makefile
	gcc -o pipe pipe.c -O0 -std=gnu99
poison.obj: poison
	objdump -D poison > poison.obj
time_mispredict_asm: time_mispredict_asm.s
	nasm -felf64 time_mispredict_asm.s -o time_mispredict_asm.o
	gcc time_mispredict_asm.o -o time_mispredict_asm
time_mispredict_c: time_mispredict_c.c Makefile
	gcc -o time_mispredict_c -std=gnu99 time_mispredict_c.c
clean: 
	rm -f poison *.o core pipe spy victim time_mispredict_asm time_mispredict_c
clean_pipes:
	rm -f /tmp/parent_to_child.pipe /tmp/child_to_parent.pipe
