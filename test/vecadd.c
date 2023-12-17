#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define USE_DOUBLE
#include "cmn_vecaddf.h"

#define N    128

double test_a[N] = {
	-10.21, -10.21, -10.21, -10.21, -10.21,
	-85.81, -85.81, -85.81, -85.81, -85.81,
	-100.001, -100.0001, -100.00001, -100.000001, -100.0000001,
	100.001, 100.0001, 100.00001, 100.000001, 100.0000001,
};

double test_b[N] = {
	10.00, 10.20, 10.21, 10.30, 10.99,
	85.00, 85.80, 85.81, 85.90, 85.99,
	0.001, 0.0001, 0.00001, 0.000001, 0.0000001,
	-0.001, -0.0001, -0.00001, -0.000001, -0.0000001,
};

double test_c[N];
double test_c_expect[N];

int test_n = N;

int main(int argc, char *argv[], char *envp[])
{
	const double *a, *b;
	double *c;
	const int *n;
	int check = 0;
	int ret = 0;

	printf("%s: test vecadd start\n", argv[0]);

	dbgprintf("argc: %d\n", argc);
	if (argc > 4) {
		a = (const double *)argv[1];
		b = (const double *)argv[2];
		c = (double *)argv[3];
		n = (const int *)argv[4];
	} else {
		/* Use test data */
		a = test_a;
		b = test_b;
		c = test_c;
		n = &test_n;
		check = 1;
	}

	dbgprintf("vector length: %d\n", *n);
	vecadd(a, b, c, *n);

	dump64(a, b, c, 10);
	if (check) {
		ret = check64(a, b, c, test_c_expect, N);
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return 0;
}
