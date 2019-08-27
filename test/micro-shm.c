#include "test-util.h"
#include <stdio.h>

int main() {
	uint64_t begin, end;
	begin = get_time();
	for (int i = 0; i < MICRO_BENCHMARK_TESTNUMS; i++) {
		// WRITE_OPS
	}
	end = get_time();
	printf("[MICRO_SHM] %lu WRITE_OPS takes %lu CPU cycles\n", MICRO_BENCHMARK_TESTNUMS, end - begin);

	begin = get_time();
	for (int i = 0; i < MICRO_BENCHMARK_TESTNUMS; i++) {
		// READ_OPS
	}
	end = get_time();
	printf("[MICRO_SHM] %lu READ_OPS takes %lu CPU cycles\n", MICRO_BENCHMARK_TESTNUMS, end - begin);
	return 0;
}
