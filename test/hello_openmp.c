#include <stdio.h>

int get_core_id(void)
{
	int ret = -1;
#if defined(__riscv_zicsr)
	__asm volatile ("csrr %0, mhartid" : "=r"(ret));
#endif
	return ret;
}

void baz(int i)
{
	printf("%d: i: %d\n", get_core_id(), i);
}

void foo(int j)
{
	int i;

	printf("parallel for loop\n");
#pragma omp parallel for
	for (i = 0; i < j; i++) {
		baz(i);
	}

	printf("parallel for loop num_threads=4\n");
#pragma omp parallel for num_threads(4)
	for (i = 0; i < j; i += 4) {
		baz(i);
	}

	printf("done\n");
}

int main(int argc, char *argv[])
{
	foo(19);
	return 0;
}

