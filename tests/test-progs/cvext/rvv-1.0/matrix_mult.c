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

    // Scalar reference calculation
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

    // RVV 1.0 assembly implementation
    __asm__ volatile(
        "li a0, 8\n"
        "vsetvli zero, a0, e64, m1\n"
        "vid.v v1\n"
        "vand.vi v2, v1, 1\n"
        "vmsne.vi v0, v2, 0\n" // v0 mask: 1 for odd indices (imaginary parts)
        "vxor.vi v5, v1, 1\n"  // v5: index swap vector [1, 0, 3, 2, 5, 4, 7, 6]
        "mv a3, %[A]\n"
        "mv a5, %[C]\n"
        "li a1, %[n]\n" // i loop counter
        "1:\n"
        "vmv.v.i v3, 0\n" // Clear accumulator for row C[i]
        "mv a4, %[B]\n"
        "mv a6, a3\n" // a6 points to A[i][k]
        "li a2, %[n]\n" // k loop counter
        "2:\n"
        "fld f0, 0(a6)\n" // A[i][k].re
        "fld f1, 8(a6)\n" // A[i][k].im
        "addi a6, a6, 16\n"
        "vle64.v v2, (a4)\n" // Load row B[k] (8 elements: 4 complex numbers)
        "vfmacc.vf v3, f0, v2\n" // v3 += a_r * [b0_r, b0_i, b1_r, b1_i, ...]
        "vrgather.vv v4, v2, v5\n" // v4 = [b0_i, b0_r, b1_i, b1_r, ...] (swapped B[k])
        "vfmacc.vf v3, f1, v4, v0.t\n" // v3(odd) += a_i * v4(odd) => v3(odd) += a_i * b_r
        "vmnot.m v0, v0\n"             // Flip mask to even indices
        "vfnmsac.vf v3, f1, v4, v0.t\n" // v3(even) -= a_i * v4(even) => v3(even) -= a_i * b_i
        "vmnot.m v0, v0\n"             // Restore mask for next k iteration
        "addi a4, a4, 64\n" // Next row of B (4 * 2 * 8 bytes)
        "addi a2, a2, -1\n"
        "bnez a2, 2b\n"
        "vse64.v v3, (a5)\n" // Store completed row C[i]
        "addi a3, a3, 64\n" // Next row of A
        "addi a5, a5, 64\n" // Next row of C
        "addi a1, a1, -1\n"
        "bnez a1, 1b\n"
        :
        : [A] "r"(A), [B] "r"(B), [C] "r"(C), [n] "i"(N)
        : "a0", "a1", "a2", "a3", "a4", "a5", "a6", "v0", "v1", "v2", "v3", "v4", "v5", "f0", "f1", "memory"
    );

    printf("%dx%d Complex Matrix Multiplication using vcfmacc.vv:\n", N, N); 
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
