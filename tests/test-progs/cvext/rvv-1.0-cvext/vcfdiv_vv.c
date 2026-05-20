#ifdef DEBUG
#include <stdio.h>
#endif
#include "../common.h"

int
main()
{
    T vs1[N_ELE] __attribute__((aligned(64)));
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64))) = {0};
    T expected[N_ELE];

    for (int i = 0; i < N_ELE / 2; i++) {
        vs1[2 * i] = (T)i + 2.0;     // c
        vs1[2 * i + 1] = (T)i + 1.5; // d
        vs2[2 * i] = (T)i + 5.0;     // a
        vs2[2 * i + 1] = (T)i + 4.0; // b
                                     //
        T a = vs2[i * 2], b = vs2[i * 2 + 1];
        T c = vs1[i * 2], d = vs1[i * 2 + 1];
        T denom = c * c + d * d;
        expected[i * 2] = ((a * c + b * d) / denom);
        expected[i * 2 + 1] = ((b * c - a * d) / denom);
    }

    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     VLD_INS " v1, (%1)\n"
                     VLD_INS " v2, (%2)\n"
                     "vcfdiv.vv v3, v2, v1\n"
                     VST_INS " v3, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(N_ELE)
                     : "a0", "v1", "v2", "v3", "memory");

    int pass = 1;
#ifdef DEBUG
    printf("vcfdiv.vv Test (VLEN=%d, ELEN=%d):\n", VLEN, ELEN);
    printf("Input vs1: ");
    for (int i = 0; i < N_ELE; i++) printf("%.2f ", (double)vs1[i]);
    printf("\nInput vs2: ");
    for (int i = 0; i < N_ELE; i++) printf("%.2f ", (double)vs2[i]);
    printf("\nExpected:  ");
    for (int i = 0; i < N_ELE; i++) printf("%.2f ", (double)expected[i]);
    printf("\nGot:       ");
    for (int i = 0; i < N_ELE; i++) printf("%.2f ", (double)vd[i]);
    printf("\n");
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-3) {
            pass = 0;
            break;
        }
    }
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
#endif
    return pass ? 0 : 1;
}
