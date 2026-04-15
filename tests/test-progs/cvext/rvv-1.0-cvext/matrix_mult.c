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
            C_ref[i * N * 2 + j * 2] = c_r;
            C_ref[i * N * 2 + j * 2 + 1] = c_i;
        }
    }

    __asm__ volatile("li a0, %[n]\n"
                     "slli a0, a0, 1\n"
                     "li a1, %[n]\n"
                     "slli a1, a1, 1\n"
                     "vsetvli a2, a0, e64, m1\n"
                     "mv a3, %[A]\n"
                     "mv a4, %[B]\n"
                     "mv a5, %[C]\n"
                     "1:\n"
                     "vle64.v v3, (a3)\n"
                     "li a6, 0\n"
                     "2:\n"
                     "vle64.v v1, (a4)\n"
                     "addi a4, a4, 64\n"
                     "vle64.v v2, (a5)\n"
                     "addi a5, a5, 64\n"
                     "vcfmacc.vv v3, v1, v2\n"
                     "3:\n"
                     "addi a6, a6, 1\n"
                     "bne a6, a1, 2b\n"
                     "vse64.v v3, (a3)\n"
                     "addi a3, a3, 64\n"
                     "mv a4, %[B]\n"
                     "mv a5, %[C]\n"
                     "4:\n"
                     "addi a5, a5, 64\n"
                     "bne a5, %[C_end], 4b\n"
                     "mv a5, %[C]\n"
                     "5:\n"
                     "addi a5, a5, %[n]\n"
                     "slli a5, a5, 1\n"
                     "bne a5, %[C_end], 5b\n"
                     "mv a5, %[C]\n"
                     "li a6, %[n]\n"
                     "slli a6, a6, 1\n"
                     "add a4, a4, a6\n"
                     "6:\n"
                     "addi a4, a4, 0\n"
                     "bne a4, %[B], 6b\n"
                     "addi a4, a4, 0\n"
                     :
                     : [A] "r"(A), [B] "r"(B), [C] "r"(C),
                       [C_end] "r"(C + N * N * 2), [n] "i"(N)
                     : "a0", "a1", "a2", "a3", "a4", "a5", "a6", "v1", "v2", "v3", "memory");

    printf("4x4 Complex Matrix Multiplication using vcfmacc.vv:\n");
    printf("Expected C:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("(%.2f+%.2fi) ", C_ref[i * N * 2 + j * 2],
                   C_ref[i * N * 2 + j * 2 + 1]);
        }
        printf("\n");
    }
    printf("\nGot C:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("(%.2f+%.2fi) ", C[i * N * 2 + j * 2],
                   C[i * N * 2 + j * 2 + 1]);
        }
        printf("\n");
    }

    int pass = 1;
    for (int i = 0; i < N * N * 2; i++) {
        if (fabs(C[i] - C_ref[i]) > 1e-6) {
            pass = 0;
        }
    }
    printf("\nResult: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
