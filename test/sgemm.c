/* Please define this macro to use CBLAS to generate expected values */
//#define USE_CBLAS

/* Please define this macro to show debug messages */
#define DEBUG_PRINT

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#if __riscv_vector == 1
#include <riscv_vector.h>

#define sgemm    sgemm_rvv
#else /* __riscv_vector == 1 */
#define sgemm    sgemm_scalar
#endif /* __riscv_vector == 1 */

#ifdef USE_CBLAS
#include <cblas.h>
#endif

#define MAX(a, b)    ((a > b) ? (a) : (b))
#define MIN(a, b)    ((a < b) ? (a) : (b))

#define M    19
#define N    17
#define K    23

#define BLK_I    64
#define BLK_J    64
#define BLK_K    64

float ta[M * K];
float tb[K * N];
float tc[M * N];

#ifdef DEBUG_PRINT
#define dbgprintf    printf
#else /* DEBUG_PRINT */
#define dbgprintf    noprintf
#endif /* DEBUG_PRINT */

static inline int __attribute__((format(printf, 1, 2))) noprintf(const char *format, ...)
{
	return 0;
}

#if __riscv_vector == 1
void sgemm_rvv(const float *a, const float *b, float *c, int mm, int nn, int kk)
{
	int vl;

	dbgprintf("----- use rvv f32\n");

	for (int j = 0; j < nn; j += vl) {
		vl = vsetvl_e32m1(nn - j);

		for (int i = 0; i < mm; i++) {
			vfloat32m1_t vc = vfmv_v_f_f32m1_ta(0.0f, vl);

			for (int k = 0; k < kk; k++) {
				vfloat32m1_t vb = vle32_v_f32m1_ta(&b[k * nn + j], vl);
				vc = vfmacc_vf_f32m1_ta(vc, a[i * kk + k], vb, vl);
			}

			vse32_v_f32m1(&c[i * nn + j], vc, vl);
		}
	}
}
#endif /* __riscv_vector == 1 */

void sgemm_scalar(const float *a, const float *b, float *c, int mm, int nn, int kk)
{
	int ie, je, ke;

	dbgprintf("----- use scalar f32\n");

	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < nn; j++) {
			c[i * nn + j] = 0.0f;
		}
	}

	for (int ib = 0; ib < mm; ib += BLK_I) {
		ie = MIN(ib + BLK_I, mm);
		for (int kb = 0; kb < kk; kb += BLK_K) {
			ke = MIN(kb + BLK_K, kk);
			for (int jb = 0; jb < nn; jb += BLK_J) {
				je = MIN(jb + BLK_J, nn);
				for (int i = ib; i < ie; i++) {
					for (int k = kb; k < ke; k++) {
						for (int j = jb; j < je; j++) {
							c[i * nn + j] += a[i * kk + k] * b[k * nn + j];
						}
					}
				}
			}
		}
	}
}

void sgemm_naive(const float *a, const float *b, float *c, int mm, int nn, int kk)
{
	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < nn; j++) {
			c[i * nn + j] = 0.0f;
			for (int k = 0; k < kk; k++) {
				c[i * nn + j] += a[i * kk + k] * b[k * nn + j];
			}
		}
	}
}

void init_test(float *a, float *b, float *c, int mm, int nn, int kk)
{
	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < kk; j++) {
			a[i * kk + j] = (float)(i * j + 10) / (mm * kk);
		}
	}
	for (int i = 0; i < kk; i++) {
		for (int j = 0; j < nn; j++) {
			b[i * nn + j] = (float)(i + j + 10) / (kk + nn);
		}
	}
	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < nn; j++) {
			c[i * nn + j] = -1.0f;
		}
	}
}

int fp_eq(float reference, float actual, float relErr)
{
	/* if near zero, do absolute error instead. */
	float absErr = relErr * ((fabsf(reference) > relErr) ? fabsf(reference) : relErr);
	return fabsf(actual - reference) < absErr;
}

int check_result(const float *c, const float *c_ex, int mm, int nn, int kk)
{
	int cnt = 0;

	for (int i = 0; i < mm; i++) {
		for (int j = 0; j < nn; j++) {
			if (!fp_eq(c_ex[i * nn + j], c[i * nn + j], 1e-6)) {
				if (cnt >= 10) {
					continue;
				}

				printf("[%d, %d]: %f != %f (expected)\n", i, j,
					c[i * nn + j], c_ex[i * nn + j]);
				cnt++;
			}
		}
	}

	return cnt;
}

int main(int argc, char *argv[], char *envp[])
{
	struct timeval st, ed, ela;
	const float *a, *b;
	float *c;
	int check = 0, m, n, k, r;
	int ret = 0;

	printf("%s: test sgemm start\n", argv[0]);
	fflush(stdout);

	dbgprintf("argc: %d\n", argc);
	if (argc > 4) {
		a = (const float *)argv[1];
		b = (const float *)argv[2];
		c = (float *)argv[3];
		m = *(int *)argv[4];
		n = *(int *)argv[4];
		k = *(int *)argv[4];
	} else {
		init_test(ta, tb, tc, M, N, K);

		a = (const float *)ta;
		b = (const float *)tb;
		c = (float *)tc;
		m = M;
		n = N;
		k = K;
		check = 1;
	}

	dbgprintf("matrix size: M:%d, N:%d, K:%d\n", m, n, k);
	gettimeofday(&st, NULL);
	sgemm(a, b, c, m, n, k);
	gettimeofday(&ed, NULL);
	timersub(&ed, &st, &ela);
	printf("time: %d.%06d\n", (int)ela.tv_sec, (int)ela.tv_usec);

#if 0
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			if (i < 4 && j < 4) {
				dbgprintf("[%d, %d]: a(%f), b(%f), c(%f)\n", i, j,
					a[i * k + j], b[i * n + j], c[i * n + j]);
			}
		}
	}
#endif

	if (check) {
		float *c_ex;

		c_ex = malloc(m * n * sizeof(float));

		gettimeofday(&st, NULL);
#ifdef USE_CBLAS
		// C = alpha AB + beta C
		float alpha = 1.0f, beta = 0.0f;
		int lda = k, ldb = n, ldc = n;

		dbgprintf("----- use CBLAS\n");
		cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
			m, n, k, alpha, a, lda, b, ldb, beta, c_ex, ldc);
#else
		dbgprintf("----- use scalar\n");
		sgemm_naive(a, b, c_ex, m, n, k);
#endif
		gettimeofday(&ed, NULL);
		timersub(&ed, &st, &ela);
		printf("verify: %d.%06d\n", (int)ela.tv_sec, (int)ela.tv_usec);

		r = check_result(c, c_ex, m, n, k);
		if (r) {
			ret = 1;
		}

		free(c_ex);
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return 0;
}
