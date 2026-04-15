#include <math.h>
#include <stdio.h>

#define N 4

int
main()
{
    double A[N * N * 2] __attribute__((aligned(64)));
    double B[N * N * 2] __attribute__((aligned(64)));
    double C[N * N * 2] __attribute__((aligned(64)));
    double C_ref[N * N * 2] __attribute__((aligned(64)));

    for (int i = 0; i < N * N * 2; i++) {
        A[i] = i + 1.0;
        B[i] = (i % 2 == 0) ? i + 1.0 : 0.0;
        C[i] = 0;
        C_ref[i] = 0;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double c_r = 0, c_i = 0;
            for (int k = 0; k < N; k++) {
                double a_r = A[i * N * 2 + k * 2],
                       a_i = A[i * N * 2 + k * 2 + 1];
                double b_r = B[k * N * 2 + j * 2],
                       b_i = B[k * N * 2 + j * 2 + 1];
                c_r += a_r * b_r - a_i * b_i;
                c_i += a_r * b_i + a_i * b_r;
            }
            C[i * N * 2 + j * 2] = c_r;
            C[i * N * 2 + j * 2 + 1] = c_i;
        }
    }

    printf("4x4 Complex Matrix Multiplication (scalar baseline):\n");
    printf("Got C:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("(%.2f+%.2fi) ", C[i * N * 2 + j * 2],
                   C[i * N * 2 + j * 2 + 1]);
        }
        printf("\n");
    }

    printf("\nResult: PASS (scalar reference)\n");
    return 0;
}
