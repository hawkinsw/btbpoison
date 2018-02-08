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
	int code = 0;
	__asm__ __volatile__ ("cpuid; rdtscp;":"=a"(lo),"=d"(hi):"a"(code):"ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

void measure(int conditional, unsigned long long *hot_time, unsigned long long *cold_time) {
	void *t = NULL;
	unsigned long long before, after;
	unsigned long long total = 0;
	int invocations = 0;
	int value = 0;

	if (conditional < 1)
		value++;
	if (conditional < 2)
		value++;
	if (conditional < 3)
		value++;
	if (conditional < 4)
		value++;
	if (conditional < 5)
		value++;
	if (conditional < 6)
		value++;
	if (conditional < 7)
		value++;
	if (conditional < 8)
		value++;
	if (conditional < 9)
		value++;
	if (conditional < 10)
		value++;
	if (conditional < 11)
		value++;
	if (conditional < 12)
		value++;
	if (conditional < 13)
		value++;
	if (conditional < 14)
		value++;
	if (conditional < 15)
		value++;
	if (conditional < 16)
		value++;
	if (conditional < 17)
		value++;
	if (conditional < 18)
		value++;
	if (conditional < 19)
		value++;
	if (conditional < 20)
		value++;
	if (conditional < 21)
		value++;
	if (conditional < 22)
		value++;
	if (conditional < 23)
		value++;
	if (conditional < 24)
		value++;
	if (conditional < 25)
		value++;
	if (conditional < 26)
		value++;
	if (conditional < 27)
		value++;
	if (conditional < 28)
		value++;
	if (conditional < 29)
		value++;
	if (conditional < 30)
		value++;

	/*
	 * Do this several times:
	 * 1. We warm up (incorrectly) the first time. 
	 * 2. We measure the hot call time.
	 * 3. We measure the cold call time.
	 */
	for (int version = 1; version<=3; version++) {
		switch (version) {
			case 1:
				t = &&T2;
				invocations = 100;
				break;
			case 2:
				t = &&T2;
				invocations = 1;
				break;
			case 3:
				t = &&T1;
				invocations = 1;
				break;
		}
	
		total = 0;
		for (int i = 0; i<invocations; i++) {
			before = rdtsc();
			asm goto("jmp *%0\n" : /*output*/: "m" (t) : /*clobbers*/:  T1, T2);


			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
			asm("nop; nop; nop;");
T1:
			after = rdtsc();
			total += (after - before);
			continue;
T2:
			after = rdtsc();
			total += (after - before);
			continue;
		}

		if (version == 2)
			*hot_time = total;
		else if (version == 3)
			*cold_time = total;
	}
}

int main() {
	cpu_set_t cpu_mask;
	int total_deltas = 250;
	float total_delta = 0.0;
	float average_delta = 0.0;

/*
	CPU_ZERO(&cpu_mask);
	CPU_SET(0, &cpu_mask);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
*/

	for (int delta = 0; delta < total_deltas; delta++)
	{
		unsigned long long total_hot = 0, total_cold = 0;
		unsigned int iterations = 0;
		unsigned int counter = 10000;

		float average_cold = 0.0;
		float average_hot = 0.0;


		for (int i = 0; i<counter; i++) {
			unsigned long long hot, cold;
			measure(0, &hot, &cold);

			total_hot += hot;
			total_cold += cold;
			iterations+=1;
		}

		average_cold = (total_cold*1.0) / (iterations);
		average_hot = (total_hot*1.0) / (iterations);
		total_delta += (average_cold-average_hot);
	}
	average_delta = (total_delta/total_deltas);

	printf("Average delta: %f\n", average_delta);
	return 0;
}
