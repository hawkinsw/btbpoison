all: poison pipe poison.obj time_mispredict_asm spy victim time_mispredict_c


EXTRA_CFLAGS=-std=gnu99 -O0
ifdef ASLR
EXTRA_CFLAGS+= -fPIC -fPIE
endif
ifdef DEBUG
EXTRA_CFLAGS+= -g
endif

spy: spyvictim.c Makefile
	gcc -o spy spyvictim.c -DSPY $(EXTRA_CFLAGS)
victim: spyvictim.c Makefile
	gcc -o victim spyvictim.c -DVICTIM $(EXTRA_CFLAGS)
poison: poison.c Makefile
	gcc -o poison poison.c $(EXTRA_CFLAGS)
	gcc -S -o poison.s poison.c
pipe: pipe.c Makefile
	gcc -o pipe pipe.c $(EXTRA_CFLAGS)
poison.obj: poison
	objdump -D poison > poison.obj
time_mispredict_asm: time_mispredict_asm.s
	nasm -felf64 time_mispredict_asm.s -o time_mispredict_asm.o
	gcc time_mispredict_asm.o -o time_mispredict_asm
time_mispredict_c: time_mispredict_c.c Makefile
	gcc -o time_mispredict_c time_mispredict_c.c $(EXTRA_CFLAGS)
clean: 
	rm -f poison *.o core pipe spy victim time_mispredict_asm time_mispredict_c poison.s
clean_pipes:
	rm -f /tmp/parent_to_child.pipe /tmp/child_to_parent.pipe
