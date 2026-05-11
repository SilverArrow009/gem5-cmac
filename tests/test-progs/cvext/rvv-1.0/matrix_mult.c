#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include "../common.h"

// #ifndef VLEN
// #define VLEN 1024
// #endif

// #ifndef ELEN
// #define ELEN 64
// #endif

// To keep it simple and ensure it fits in one vector:
#define N (VLEN / (2 * ELEN))
#if N < 1
#undef N
#define N 1
#endif

// #if ELEN == 64
// typedef double T;
// #define VSET_E "e64"
// #define VLD_INS "vle64.v"
// #define VST_INS "vse64.v"
// #define LD_INS "fld"
// #elif ELEN == 32
// typedef float T;
// #define VSET_E "e32"
// #define VLD_INS "vle32.v"
// #define VST_INS "vse32.v"
// #define LD_INS "flw"
// #elif ELEN == 16
// typedef _Float16 T;
// #define VSET_E "e16"
// #define VLD_INS "vle16.v"
// #define VST_INS "vse16.v"
// #define LD_INS "flh"
// #elif ELEN == 8
// typedef int8_t T;
// #define VSET_E "e8"
// #define VLD_INS "vle8.v"
// #define VST_INS "vse8.v"
// #define LD_INS "lb"
// #endif

int
main()
{
    T A[N * N * 2] __attribute__((aligned(64)));
    T B[N * N * 2] __attribute__((aligned(64)));
    T C[N * N * 2] __attribute__((aligned(64)));
    T C_ref[N * N * 2] __attribute__((aligned(64)));

    for (int i = 0; i < N * N * 2; i++) {
        A[i] = (T)(i + 1.0);
        B[i] = (T)((i % 2 == 0) ? i + 1.0 : 0.0);
        C[i] = (T)0;
        C_ref[i] = (T)0;
    }

    // Scalar reference calculation
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double c_r = 0, c_i = 0;
            for (int k = 0; k < N; k++) {
                double a_r = (double)A[i * N * 2 + k * 2],
                       a_i = (double)A[i * N * 2 + k * 2 + 1];
                double b_r = (double)B[k * N * 2 + j * 2],
                       b_i = (double)B[k * N * 2 + j * 2 + 1];
                c_r += a_r * b_r - a_i * b_i;
                c_i += a_r * b_i + a_i * b_r;
            }
            C_ref[i * N * 2 + j * 2] = (T)c_r;
            C_ref[i * N * 2 + j * 2 + 1] = (T)c_i;
        }
    }

    int n_ele = 2 * N;
    int stride = n_ele * sizeof(T);

    // RVV 1.0 assembly implementation
    __asm__ volatile(
        "mv a0, %[n_ele]\n"
        "vsetvli zero, a0, " VSET_E ", m1\n"
        "vid.v v1\n"
        "vand.vi v2, v1, 1\n"
        "vmsne.vi v0, v2, 0\n" // v0 mask: 1 for odd indices (imaginary parts)
        "vxor.vi v5, v1, 1\n"  // v5: index swap vector [1, 0, 3, 2, ...]

        "mv a3, %[A_ptr]\n"
        "mv a5, %[C_ptr]\n"
        "li a1, %[n]\n" // i loop counter
        "1:\n"
        "vmv.v.i v3, 0\n" // Clear accumulator for row C[i]
        "mv a4, %[B_ptr]\n"
        "mv a6, a3\n" // a6 points to A[i][k]
        "li a2, %[n]\n" // k loop counter
        "2:\n"
        LD_INS " f0, 0(a6)\n" // A[i][k].re
        LD_INS " f1, %[ele_size](a6)\n" // A[i][k].im
        "addi a6, a6, %[ele_size_2]\n"

        VLD_INS " v2, (a4)\n" // Load row B[k] (n_ele elements: N complex numbers)

        // Complex multiply-accumulate using standard RVV 1.0:
        // C += (a_r + i*a_i) * (b_r + i*b_i)
        // Re(C) += a_r*b_r - a_i*b_i
        // Im(C) += a_r*b_i + a_i*b_r

        "vfmacc.vf v3, f0, v2\n" // v3 += a_r * [b0_r, b0_i, b1_r, b1_i, ...]

        "vrgather.vv v4, v2, v5\n" // v4 = [b0_i, b0_r, b1_i, b1_r, ...] (swapped B[k])

        "vfmacc.vf v3, f1, v4, v0.t\n" // v3(odd) += a_i * v4(odd) => v3(odd) += a_i * b_r
        "vmnot.m v0, v0\n"             // Flip mask to even indices
        "vfnmsac.vf v3, f1, v4, v0.t\n" // v3(even) -= a_i * v4(even) => v3(even) -= a_i * b_i
        "vmnot.m v0, v0\n"             // Restore mask for next k iteration

        "add a4, a4, %[stride]\n" // Next row of B
        "addi a2, a2, -1\n"
        "bnez a2, 2b\n"

        VST_INS " v3, (a5)\n" // Store completed row C[i]
        "add a3, a3, %[stride]\n" // Next row of A
        "add a5, a5, %[stride]\n" // Next row of C
        "addi a1, a1, -1\n"
        "bnez a1, 1b\n"
        :
        : [A_ptr] "r"(A), [B_ptr] "r"(B), [C_ptr] "r"(C), [n] "i"(N),
          [n_ele] "r"(n_ele), [stride] "r"(stride),
          [ele_size] "i"(sizeof(T)), [ele_size_2] "i"(2 * sizeof(T))
        : "a0", "a1", "a2", "a3", "a4", "a5", "a6", "v0", "v1", "v2", "v3", "v4", "v5", "f0", "f1", "memory"
    );

    printf("%dx%d Complex Matrix Multiplication using standard RVV 1.0 (VLEN=%d, ELEN=%d):\n", N, N, VLEN, ELEN);
    printf("Expected C:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("(%.2f+%.2fi) ", (double)C_ref[i * N * 2 + j * 2],
                   (double)C_ref[i * N * 2 + j * 2 + 1]);
        }
        printf("\n");
    }
    printf("\nGot C:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("(%.2f+%.2fi) ", (double)C[i * N * 2 + j * 2],
                   (double)C[i * N * 2 + j * 2 + 1]);
        }
        printf("\n");
    }

    int pass = 1;
    for (int i = 0; i < N * N * 2; i++) {
        if (fabs((double)C[i] - (double)C_ref[i]) > 1e-3) {
            pass = 0;
        }
    }
    printf("\nResult: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
