all: poison pipe poison.obj time_mispredict_asm spy victim time_mispredict_c spy.obj victim.obj time_jump attack attack.obj

EXTRA_CFLAGS=-std=gnu99 -O0
EXTRA_CXXFLAGS=-std=c++11 -O0
ifdef ASLR
EXTRA_CFLAGS+= -fPIC -fPIE
EXTRA_CXXFLAGS+= -fPIC -fPIE
endif
ifdef DEBUG
EXTRA_CFLAGS+= -g
EXTRA_CXXFLAGS+= -g
endif

spy: spyvictim.c Makefile
	gcc -o spy.s -S spyvictim.c -DSPY $(EXTRA_CFLAGS)
	gcc -o spy spyvictim.c -DSPY $(EXTRA_CFLAGS)
victim: spyvictim.c Makefile
	gcc -o victim spyvictim.c -DVICTIM $(EXTRA_CFLAGS)
spy.obj: spy
	objdump -D spy > spy.obj
victim.obj: victim
	objdump -D victim > victim.obj

time_jump: time_jump.c Makefile
	gcc -o time_jump time_jump.c $(EXTRA_CFLAGS)

attack: attack.c Makefile
	gcc -o attack attack.c $(EXTRA_CFLAGS)
	gcc -S -o attack.s attack.c $(EXTRA_CFLAGS)
attack.obj: attack
	objdump -D attack > attack.obj

poison: poison.c Makefile
ifndef MSR
	gcc -o poison poison.c $(EXTRA_CFLAGS)
	gcc -S -o poison.s poison.c $(EXTRA_CFLAGS)
else
	g++ -o poison poison.c -DUSEMSR -I../libmsr/ -L../libmsr/ -lmsr $(EXTRA_CXXFLAGS)
endif
poison.obj: poison
	objdump -D poison > poison.obj

pipe: pipe.c Makefile
	gcc -o pipe pipe.c $(EXTRA_CFLAGS)

time_mispredict_asm: time_mispredict_asm.s
	nasm -felf64 time_mispredict_asm.s -o time_mispredict_asm.o
	gcc time_mispredict_asm.o -o time_mispredict_asm
time_mispredict_c: time_mispredict_c.c Makefile
	gcc -o time_mispredict_c time_mispredict_c.c $(EXTRA_CFLAGS)

clean: 
	rm -f poison *.o core pipe spy victim time_mispredict_asm time_mispredict_c poison.s victim.obj spy.obj time_jump attack.obj attack
clean_pipes:
	rm -f /tmp/parent_to_child.pipe /tmp/child_to_parent.pipe
