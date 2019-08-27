#ifndef __TEST_UTIL_H
#define __TEST_UTIL_H
#include <stdint.h>
#define MICRO_BENCHMARK_TESTNUMS 1000000
static inline uint64_t get_time() {
	uint32_t lo, hi;
	__asm__ volatile("rdtsc\n\t"
		:"=a"(lo), "=d"(hi)
		:
		:);
	return (uint64_t)hi << 32 | lo;
}
#endif
