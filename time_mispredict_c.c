#include <stdio.h>

static inline __attribute__((always_inline)) unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("mfence; rdtscp;" : "=a"(lo), "=d"(hi) : : "ecx");
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

/*
 * Measure the time for a predicted (hot) and a 
 * mispredicted jmp.
 *
 * Input: loop_max
 * loop_max - 3 iterations will warm up the predictor.
 * iteration loop_max - 2 is measured for the predicted jmp time.
 * iteration loop_max - 1 is measured for the mispredicted jmp time.
 *
 * Output:
 * hot_total - time for the predicted jmp to execute.
 * cold_total - time for the mispredicted jmp to execute.
 */
void measure(int loop_max, unsigned long long *hot_total, unsigned long long *cold_total) {
	int warmup_count = loop_max - 3;
	int measure_hot_iteration = loop_max - 2;
	int measure_cold_iteration = loop_max - 1;
	int value = 0;
	unsigned long long before, after; 

	for (int i = 0; i<loop_max; i++) {

		int comparison = 0;

		if (i<=measure_hot_iteration)
			comparison = 1;
		else if (i==measure_cold_iteration)
			comparison = 2;

		before = rdtsc();

		if (comparison == 1) {
			value += 1;
		} else {
			value += 2;
		}
		after = rdtsc();

		if (measure_hot_iteration == i) {
			*hot_total = (after - before);
		} else if (measure_cold_iteration == i) {
			*cold_total = (after-before);
		}
	}
}

int main() {
	int iteration_count = 15;
	unsigned long long hot_time = 0, cold_time = 0;
	unsigned long long hot_total= 0, cold_total = 0;

	measure(iteration_count, &hot_time, &cold_time);
	cold_total += cold_time;
	hot_total += hot_time;

	printf("cold average : %llu\n", cold_total);
	printf("hot average  : %llu\n", hot_total);
	return 1;
}
