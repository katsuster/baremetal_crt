#include <math.h>
#include <stdio.h>

#include "cmn_vecaddf.h"

#if __riscv_vector == 1
void vecadd32_rvv(const float *a, const float *b, float *c, int n)
{
	vfloat32m1_t va, vb, vc;
	size_t l;

	dbgprintf("----- use rvv f32\n");

	for (; n > 0; n -= l) {
		l = vsetvl_e32m1(n);
		va = vle32_v_f32m1(a, l);
		a += l;
		vb = vle32_v_f32m1(b, l);
		b += l;
		vc = vfadd_vv_f32m1(va, vb, l);
		vse32_v_f32m1(c, vc, l);
		c += l;
	}
}

void vecadd64_rvv(const double *a, const double *b, double *c, int n)
{
	vfloat64m1_t va, vb, vc;
	size_t l;

	dbgprintf("----- use rvv f64\n");

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

void vecadd32_scalar(const float *a, const float *b, float *c, int n)
{
	dbgprintf("----- use scalar f32\n");

	for (int i = 0; i < n; i++) {
		c[i] = a[i] + b[i];
	}
}

void vecadd64_scalar(const double *a, const double *b, double *c, int n)
{
	dbgprintf("----- use scalar f64\n");

	for (int i = 0; i < n; i++) {
		c[i] = a[i] + b[i];
	}
}

int fp32_eq(float reference, float actual, float relErr)
{
	/* if near zero, do absolute error instead. */
	float absErr = relErr * ((fabsf(reference) > relErr) ? fabsf(reference) : relErr);
	return fabsf(actual - reference) < absErr;
}

int fp64_eq(double reference, double actual, double relErr)
{
	/* if near zero, do absolute error instead. */
	double absErr = relErr * ((fabs(reference) > relErr) ? fabs(reference) : relErr);
	return fabs(actual - reference) < absErr;
}

int check32(const float *a, const float *b, const float *c, float *expect, int n)
{
	int ret = 0;

	vecadd32_scalar(a, b, expect, n);

	for (int i = 0; i < n; i++) {
		if (!fp32_eq(c[i], expect[i], 1e-6)) {
			printf("failed, %f=!%f (expected)\n", c[i], expect[i]);
			ret = -1;
		}
	}

	return ret;
}

int check64(const double *a, const double *b, const double *c, double *expect, int n)
{
	int ret = 0;

	vecadd64_scalar(a, b, expect, n);

	for (int i = 0; i < n; i++) {
		if (!fp32_eq(c[i], expect[i], 1e-6)) {
			printf("failed, %f=!%f (expected)\n", c[i], expect[i]);
			ret = -1;
		}
	}

	return ret;
}

void dump32(const float *a, const float *b, const float *c, int n)
{
	for (int i = 0; i < n; i++) {
		dbgprintf("%d: a(%f) + b(%f) = c(%f)\n", i, a[i], b[i], c[i]);
	}
}

void dump64(const double *a, const double *b, const double *c, int n)
{
	for (int i = 0; i < n; i++) {
		dbgprintf("%d: a(%f) + b(%f) = c(%f)\n", i, a[i], b[i], c[i]);
	}
}
