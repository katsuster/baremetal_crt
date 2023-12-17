/*
 * Base code and test data come from
 * https://github.com/riscv-non-isa/rvv-intrinsic-doc/blob/master/examples/rvv_saxpy.c
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#if __riscv_vector == 1
#include <riscv_vector.h>

#define daxpy    daxpy_rvv
#else /* __riscv_vector == 1 */
#define daxpy    daxpy_scalar
#endif /* __riscv_vector == 1 */

#define N    128

double test_a = 55.66;

double test_x[N] = {
	-0.4325648115282207, -1.6655843782380970, 0.1253323064748307,
	0.2876764203585489,  -1.1464713506814637, 1.1909154656429988,
	1.1891642016521031,  -0.0376332765933176, 0.3272923614086541,
	0.1746391428209245,  -0.1867085776814394, 0.7257905482933027,
	-0.5883165430141887, 2.1831858181971011,  -0.1363958830865957,
	0.1139313135208096,  1.0667682113591888,  0.0592814605236053,
	-0.0956484054836690, -0.8323494636500225, 0.2944108163926404,
	-1.3361818579378040, 0.7143245518189522,  1.6235620644462707,
	-0.6917757017022868, 0.8579966728282626,  1.2540014216025324,
	-1.5937295764474768, -1.4409644319010200, 0.5711476236581780,
	-0.3998855777153632
};

double test_y[N] = {
	1.7491401329284098,  0.1325982188803279,  0.3252281811989881,
	-0.7938091410349637, 0.3149236145048914,  -0.5272704888029532,
	0.9322666565031119,  1.1646643544607362,  -2.0456694357357357,
	-0.6443728590041911, 1.7410657940825480,  0.4867684246821860,
	1.0488288293660140,  1.4885752747099299,  1.2705014969484090,
	-1.8561241921210170, 2.1343209047321410,  1.4358467535865909,
	-0.9173023332875400, -1.1060770780029008, 0.8105708062681296,
	0.6985430696369063,  -0.4015827425012831, 1.2687512030669628,
	-0.7836083053674872, 0.2132664971465569,  0.7878984786088954,
	0.8966819356782295,  -0.1869172943544062, 1.0131816724341454,
	0.2484350696132857
};

double test_y_expect[N] = {
	1.7491401329284098,  0.1325982188803279,  0.3252281811989881,
	-0.7938091410349637, 0.3149236145048914,  -0.5272704888029532,
	0.9322666565031119,  1.1646643544607362,  -2.0456694357357357,
	-0.6443728590041911, 1.7410657940825480,  0.4867684246821860,
	1.0488288293660140,  1.4885752747099299,  1.2705014969484090,
	-1.8561241921210170, 2.1343209047321410,  1.4358467535865909,
	-0.9173023332875400, -1.1060770780029008, 0.8105708062681296,
	0.6985430696369063,  -0.4015827425012831, 1.2687512030669628,
	-0.7836083053674872, 0.2132664971465569,  0.7878984786088954,
	0.8966819356782295,  -0.1869172943544062, 1.0131816724341454,
	0.2484350696132857
};

int test_n = N;

#if __riscv_vector == 1
void daxpy_rvv(const double a, const double *x, double *y, int n)
{
	vfloat64m1_t vx, vy;
	size_t l;

	printf("----- use rvv f64\n");

	for (; n > 0; n -= l) {
		l = vsetvl_e64m1(n);
		vx = vle64_v_f64m1(x, l);
		x += l;
		vy = vle64_v_f64m1(y, l);
		vy = vfmacc_vf_f64m1(vy, a, vx, l);
		vse64_v_f64m1(y, vy, l);
		y += l;
	}
}
#endif /* __riscv_vector == 1 */

void daxpy_scalar(const double a, const double *x, double *y, int n)
{
	printf("----- use scalar f64\n");

	for (int i = 0; i < n; i++) {
		y[i] = a * x[i] + y[i];
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
	double *a, *x, *y;
	int *n, check = 0;
	int ret = 0;

	printf("%s: test daxpy start\n", argv[0]);

	printf("argc: %d\n", argc);
	if (argc > 4) {
		a = (double *)argv[1];
		x = (double *)argv[2];
		y = (double *)argv[3];
		n = (int *)argv[4];
	} else {
		/* Use test data */
		a = &test_a;
		x = test_x;
		y = test_y;
		n = &test_n;
		check = 1;
	}

	printf("vector length: %d\n", *n);
	daxpy(*a, x, y, *n);

	for (int i = 0; i < *n; i++) {
		if (i < 10) {
			printf("%d: a(%f), x(%f), y(%f)\n", i, *a, x[i], y[i]);
		}
	}

	if (check) {
		daxpy_scalar(*a, x, test_y_expect, *n);

		for (int i = 0; i < N; i++) {
			if (!fp_eq(test_y_expect[i], y[i], 1e-6)) {
				printf("failed, %f=!%f\n", test_y_expect[i], y[i]);
				ret = 1;
			}
		}
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return 0;
}
