#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

extern int errno;

static inline __attribute__((always_inline)) unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtscp;" : "=a"(lo), "=d"(hi) : : "ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

unsigned long long cross(int target, int loop_max) {
	register void *t asm ("r12") = NULL;
	int value = 0;
	volatile float result = 0.0;
	unsigned long long before, after;
	unsigned long long delta = 0;

	switch (target) {
		case 1:
			t = &&T1;
			break;
		case 2:
			t = &&T2;
			break;
		}

	for (int i = 0; i<loop_max; i++)
	{
		before = rdtsc();
		asm goto("jmp *%%r12\n" : /*output*/:/*input*/:/*clobbers*/:  T1, T2);
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
		asm ("nop; nop; nop; nop;\n");
T1:
		after = rdtsc();
		delta = (after - before);
		continue;
T2:
		after = rdtsc();
		delta = (after - before);
		continue;
	}
	return delta;
}

int main() {
	int child = 0;
	int target_max = 4;

	int child_to_parent_pipe[2] = {0,0};
	int parent_to_child_pipe[2] = {0,0};
	int p2c_write, p2c_read, c2p_write, c2p_read;
	char just_read = '\n';
	char to_write = 'W';

	unsigned long long totals = 0, fake = 0;
	unsigned int iterations = 0;
	unsigned int inner_iterations = 10;
	unsigned int counter = 1;

	float average = 0.0;

	cpu_set_t cpu_mask;

	CPU_ZERO(&cpu_mask);
	CPU_SET(0, &cpu_mask);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);

	if (pipe(child_to_parent_pipe) || pipe(parent_to_child_pipe)) {
		printf("Error with pipe(): %s\n", strerror(errno));
		return 1;
	}

	totals = cross(1,1);
	printf("uncached time jump to 1: %llu\n", totals);
	cross(1,1000);
	totals = cross(1,1);
	printf("  cached time jump to 1: %llu\n", totals);

	totals = cross(2,1);
	printf("uncached time jump to 2: %llu\n", totals);
	cross(2,1000);
	totals = cross(2,1);
	printf("  cached time jump to 2: %llu\n", totals);

	totals = 0.0;

	c2p_read = child_to_parent_pipe[0];
	c2p_write= child_to_parent_pipe[1];

	p2c_read = parent_to_child_pipe[0];
	p2c_write= parent_to_child_pipe[1];

	if (!(child = fork())) {
		for (int i = 0; i<counter; i++) {
			read(p2c_read, &just_read, 1);

			totals += cross(1, 25);

			write(c2p_write, &to_write, 1);

			iterations += 1;
		}
		average = (totals*1.0) / iterations;
		printf("child time per iteration: %f\n", average);
		return 0;
	}

	for (int i = 0; i<counter; i++) {
		/*
		 * This is optional.
		fake = cross(1, 25);
		 */
		write(p2c_write, &to_write, 1);
		read(c2p_read, &just_read, 1);
		totals += cross(2, 1);

		iterations+=1;
	}

	average = (totals*1.0) / iterations;
	printf("parent time per iteration: %f\n", average);


	waitpid(child, NULL, 0);

	return 0;
}
