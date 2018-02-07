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
#include <fcntl.h>
#include <sys/stat.h>

extern int errno;

static inline __attribute__((always_inline)) unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtscp;" : "=a"(lo), "=d"(hi) : : "ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

unsigned long long cross(int loop_max) {
	unsigned long long before, after;
	unsigned long long total = 0;

	for (int i = 0; i<loop_max; i++)
	{
		before = rdtsc();
#ifdef VICTIM
		asm goto("jmp %l0\n" : /*output*/: /*input*/ : /*clobbers*/:  T2);
#elif SPY
		asm goto("jmp %l0\n" : /*output*/: /*input*/ : /*clobbers*/:  T1);
#else
#error VICTIM or SPY must be defined.
#endif
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
	asm("nop; nop; nop; nop;");
T1:
		after = rdtsc();
		total += (after - before);
		continue;
T2:
		after = rdtsc();
		total += (after - before);
		continue;
	}
	return total;
}

int main() {
	int child = 0;
	int target_max = 4;
	int v = 1;

	int p2c, c2p;

	char just_read = '\n';
	char to_write = 'W';

	unsigned long long totals = 0;
	unsigned int iterations = 0;
	unsigned int inner_iterations = 10;
	unsigned int counter = 100000;

	float average = 0.0;

	cpu_set_t cpu_mask;

	CPU_ZERO(&cpu_mask);
	CPU_SET(0, &cpu_mask);
	//sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);

#define P2C_FILENAME "/tmp/parent_to_child.pipe"
#define C2P_FILENAME "/tmp/child_to_parent.pipe"
/*
 * Victim creates the FIFOs.
 */

#ifdef VICTIM
	if (v)
#else
	if (!v)
#endif
	if (mkfifo(P2C_FILENAME, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) ||
	    mkfifo(C2P_FILENAME, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) {
		/*
		 * Error occurred creating the FIFO pipe.
		 */
		return 1;
	}

	if (((c2p= open(C2P_FILENAME, O_RDWR)) < 0) ||
	    ((p2c= open(P2C_FILENAME, O_RDWR)) < 0)) {
		/*
		 * error opening the files!
		 */
		return 1;
	}

#ifdef VICTIM
	for (int i = 0; i<counter; i++) {
		read(p2c, &just_read, 1);

		printf("Executing cross in child (1, 25)\n");
		totals += cross(50);

		write(c2p, &to_write, 1);

		iterations += 50;
	}
	average = totals / iterations;
	printf("victim time per iteration: %f\n", average);
#elif SPY
	for (int i = 0; i<counter; i++) {
		write(p2c, &to_write, 1);
		read(c2p, &just_read, 1);
		printf("Executing cross in parent (2, 1)\n");
		totals += cross(1);

		iterations+=1;
	}
	average = totals / iterations;
	printf("spy time per iteration: %f\n", average);
#endif

	close(p2c);
	close(c2p);

#ifdef VICTIM
	if (v)
#else
	if (!v)
#endif
	if (unlink(C2P_FILENAME) || unlink(P2C_FILENAME)) {
		/*
		 * Error unlinking the fifo!
		 */
		return 1;
	}

	return 0;
}
