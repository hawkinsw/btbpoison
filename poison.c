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

static inline __attribute__((always_inline)) unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtscp;" : "=a"(lo), "=d"(hi) : : "ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

unsigned long long cross(int target,
	int loop_max,
	const char *context
#ifdef USEMSR
	,
	unsigned long long *misses
#endif
	) {
	register void *t asm ("r12") = NULL;
	unsigned long long before, after;
	unsigned long long delta = 0;

#ifdef USEMSR
	Msr msr;
	uint64_t sel0 = 0x0;
	uint64_t sel1 = 0x0;
	uint64_t perf_ctr = 0x0;
	uint64_t msr_result = 0x0;
	uint64_t zero = 0x0;

	*misses = 0;
#endif



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

#ifdef USEMSR
	perf_ctr = MSR::PERF_EVTSEL0_ENABLE | MSR::PERF_EVTSEL1_ENABLE;

	sel0 = MSR::EVT_BR_MISP_RETIRED |
	       MSR::MASK_BR_MISP_RETIRED_ALL |
	       MSR::EVENT_ENABLE |
	       MSR::EVENT_USER_ENABLE;

	sel1 = MSR::EVT_BR_INST_RETIRED |
	       MSR::MASK_BR_INST_RETIRED_ALL |
	       MSR::EVENT_ENABLE |
	       MSR::EVENT_USER_ENABLE;

	assert(msr.Write(MSR::MSR_CORE_PERF_GLOBAL_CTRL, zero, CPU_NO) == 0);
	assert(msr.Write(MSR::MSR_IA32_PERFEVTSEL0, sel0, CPU_NO) == 0);
	assert(msr.Write(MSR::MSR_IA32_PERFEVTSEL1, sel1, CPU_NO) == 0);
	assert(msr.Write(MSR::MSR_IA32_PMC0, zero, CPU_NO) == 0);
	assert(msr.Write(MSR::MSR_IA32_PMC1, zero, CPU_NO) == 0);
	(msr.Write(MSR::MSR_CORE_PERF_GLOBAL_CTRL, perf_ctr, CPU_NO) == 0);
#endif

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
#ifdef USEMSR
	(msr.Write(MSR::MSR_CORE_PERF_GLOBAL_CTRL, zero, CPU_NO) == 0);
	assert(msr.Write(MSR::MSR_IA32_PERFEVTSEL0, zero, CPU_NO) == 0);

	assert(msr.Read(MSR::MSR_IA32_PMC1, msr_result, CPU_NO) == 0);
	//std::cout << context << " branch count: " << std::dec << msr_result << std::endl;
	assert(msr.Read(MSR::MSR_IA32_PMC0, msr_result, CPU_NO) == 0);
	//std::cout << context << " missed branch count: " << std::dec << msr_result << std::endl;
	*misses = msr_result;
#endif
	delta = (after - before);
		continue;
T2:
		after = rdtsc();
#ifdef USEMSR
	assert(msr.Write(MSR::MSR_CORE_PERF_GLOBAL_CTRL, zero, CPU_NO) == 0);
	assert(msr.Write(MSR::MSR_IA32_PERFEVTSEL0, zero, CPU_NO) == 0);

	assert(msr.Read(MSR::MSR_IA32_PMC1, msr_result, CPU_NO) == 0);
	//std::cout << context << " branch count: " << std::dec << msr_result << std::endl;
	assert(msr.Read(MSR::MSR_IA32_PMC0, msr_result, CPU_NO) == 0);
	//std::cout << context << " missed branch count: " << std::dec << msr_result << std::endl;
	*misses = msr_result;
#endif
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
	unsigned long long misses = 0;

	float average = 0.0;

	cpu_set_t cpu_mask;

	CPU_ZERO(&cpu_mask);
	CPU_SET(CPU_NO, &cpu_mask);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);

	if (pipe(child_to_parent_pipe) || pipe(parent_to_child_pipe)) {
		printf("Error with pipe(): %s\n", strerror(errno));
		return 1;
	}

#if 0
	totals = cross(1,1, "None");
	printf("uncached time jump to 1: %llu\n", totals);
	cross(1,1000, "None");
	totals = cross(1,1, "None");
	printf("  cached time jump to 1: %llu\n", totals);

	totals = cross(2,1, "None");
	printf("uncached time jump to 2: %llu\n", totals);
	cross(2,1000, "None");
	totals = cross(2,1, "None");
	printf("  cached time jump to 2: %llu\n", totals);
#endif

	totals = 0.0;

	c2p_read = child_to_parent_pipe[0];
	c2p_write= child_to_parent_pipe[1];

	p2c_read = parent_to_child_pipe[0];
	p2c_write= parent_to_child_pipe[1];


	if (!(child = fork())) {
		for (int i = 0; i<counter; i++) {
			read(p2c_read, &just_read, 1);

#ifndef USEMSR
			totals += cross(2, 100, "Child");
#else
			totals += cross(2, 100, "Child", &misses);
#endif
			write(c2p_write, &to_write, 1);

			iterations += 1;
		}
#ifndef USEMSR
		average = (totals*1.0) / iterations;
		printf("child time per iteration: %f\n", average);
#else
		average = (misses*1.0) / (iterations);
		printf("child misses per iteration: %f\n", average);
#endif
		return 0;
	}

	for (int i = 0; i<counter; i++) {
		/*
		 * This is optional.
		fake = cross(2, 25);
		 */
		write(p2c_write, &to_write, 1);
		read(c2p_read, &just_read, 1);
#ifndef USEMSR
		totals += cross(1, 1, "Parent");
#else
		totals += cross(1, 1, "Parent", &misses);
#endif
		iterations+=1;
	}

#ifndef USEMSR
	average = (totals*1.0) / iterations;
	printf("parent time per iteration: %f\n", average);
#else
	average = (misses*1.0) / iterations;
	printf("parent misses per iteration: %f\n", average);
#endif

	waitpid(child, NULL, 0);

	return 0;
}
