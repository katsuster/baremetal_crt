#include <math.h>
#include <stdint.h>
#include <stdio.h>

#if __riscv_vector == 1
#include <riscv_vector.h>

#define vecadd    vecadd_rvv
#else /* __riscv_vector == 1 */
#define vecadd    vecadd_scalar
#endif /* __riscv_vector == 1 */

#if __riscv_vector == 1
#endif /* __riscv_vector == 1 */

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

#if __riscv_vector == 1
void vecadd_rvv(const double *a, const double *b, double *c, int n)
{
	vfloat64m1_t va, vb, vc;
	size_t l;

	printf("----- use rvv f64\n");

	for (; n > 0; n -= l) {
		l = vsetvl_e64m1(n);
		va = vle64_v_f64m1(a, l);
		a += l;
		vb = vle64_v_f64m1(b, l);
		b += l;
		vc = vfadd_vv_f64m1(va, vb, l);
		vse64_v_f64m1(c, vc, l);
		c += l;
	}
}
#endif /* __riscv_vector == 1 */

void vecadd_scalar(const double *a, const double *b, double *c, int n)
{
	printf("----- use scalar f64\n");

	for (int i = 0; i < n; i++) {
		c[i] = a[i] + b[i];
	}
}

int fp_eq(double reference, double actual, double relErr)
{
	/* if near zero, do absolute error instead. */
	double absErr = relErr * ((fabs(reference) > relErr) ? fabs(reference) : relErr);
	return fabs(actual - reference) < absErr;
}

int main(int argc, char *argv[], char *envp[])
{
	double *a, *b, *c;
	int *n, check = 0;

	printf("%s: vecadd start\n", argv[0]);

	printf("argc: %d\n", argc);
	if (argc > 4) {
		a = (double *)argv[1];
		b = (double *)argv[2];
		c = (double *)argv[3];
		n = (int *)argv[4];
	} else {
		/* Use test data */
		a = test_a;
		b = test_b;
		c = test_c;
		n = &test_n;
		check = 1;
	}

	printf("vector length: %d\n", *n);
	vecadd(a, b, c, *n);

	for (int i = 0; i < *n; i++) {
		if (i < 10) {
			printf("%d: a(%f) + b(%f) = c(%f)\n", i, a[i], b[i], c[i]);
		}
	}

	if (check) {
		int pass = 1;

		vecadd_scalar(a, b, test_c_expect, *n);

		for (int i = 0; i < N; i++) {
			if (!fp_eq(test_c_expect[i], c[i], 1e-6)) {
				printf("failed, %f=!%f\n", test_c_expect[i], c[i]);
				pass = 0;
			}
		}
		if (pass) {
			printf("passed\n");
		}
	}

	return 0;
}
