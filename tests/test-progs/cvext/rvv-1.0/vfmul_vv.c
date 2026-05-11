#include <stdio.h>
#include <math.h>
#include "../common.h"

int
main()
{
    T vs1[N_ELE] __attribute__((aligned(64)));
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64))) = {0};
    T expected[N_ELE];

    for (int i = 0; i < N_ELE; i++) {
        vs1[i] = (T)(i + 1.0);
        vs2[i] = (T)((i % 2 == 0) ? (i / 2 + 1.0) : 1.0);
    }

    for (int i = 0; i < N_ELE / 2; i++) {
        double a = (double)vs1[i * 2], b = (double)vs1[i * 2 + 1];
        double c = (double)vs2[i * 2], d = (double)vs2[i * 2 + 1];
        printf("%f, %f, %f, %f\n", a, b, c, d);
        expected[i * 2] = (T)(a * c - b * d);
        expected[i * 2 + 1] = (T)(a * d + b * c);
    }

    int n_complex = N_ELE/2;
    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     VLD2_INS " v10, (%1)\n" // v10=re1, v11=im1
                     VLD2_INS " v12, (%2)\n" // v12=re2, v13=im2

                     // Real: re1*re2 - im1*im2
                     "vfmul.vv v14, v10, v12\n"
                     "vfnmsac.vv v14, v11, v13\n"

                     // Imag: re1*im2 + im1*re2
                     "vfmul.vv v15, v10, v13\n"
                     "vfmacc.vv v15, v11, v12\n"

                     VST2_INS " v14, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(n_complex)
                     : "a0", "v10", "v11", "v12", "v13", "v14", "v15", "memory");

    printf("vfmul.vv (standard RVV 1.0 complex) Test (VLEN=%d, ELEN=%d):\n", VLEN, ELEN);
    printf("Input vs1: ");
    for (int i = 0; i < N_ELE; i++) {
        printf("%.2f ", vs1[i]);
    }
    printf("\n");
    printf("Input vs2: ");
    for (int i = 0; i < N_ELE; i++) {
        printf("%.2f ", vs2[i]);
    }
    printf("\n");
    printf("Expected:  ");
    for (int i = 0; i < N_ELE; i++) {
        printf("%.2f ", expected[i]);
    }
    printf("\n");
    printf("Got:       ");
    for (int i = 0; i < N_ELE; i++) {
        printf("%.2f ", vd[i]);
    }
    printf("\n");

    int pass = 1;
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-3) {
            pass = 0;
        }
    }
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
