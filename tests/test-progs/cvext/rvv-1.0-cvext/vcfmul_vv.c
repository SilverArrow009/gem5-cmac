#include <stdio.h>
#include "../common.h"

int
main()
{
    T vs1[N_ELE] __attribute__((aligned(64)));
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64)));
    T expected[N_ELE];

    for (int i = 0; i < N_ELE; i++) {
        vs1[i] = (T)(i + 1.0);
        vs2[i] = (T)((i % 2 == 0) ? (i / 2 + 1.0) : 1.0);
        vd[i] = (T)0.0;
    }

    for (int i = 0; i < N_ELE / 2; i++) {
        double a = (double)vs1[i * 2];
        double b = (double)vs1[i * 2 + 1];
        double c = (double)vs2[i * 2];
        double d = (double)vs2[i * 2 + 1];
        expected[i * 2] = (T)(a * c - b * d);
        expected[i * 2 + 1] = (T)(a * d + b * c);
        printf("Debug: expected[%d]=%f, expected[%d]=%f\n", i*2, (double)expected[i*2], i*2+1, (double)expected[i*2+1]);
    }

    int n = N_ELE;
    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     VLD_INS " v1, (%1)\n"
                     VLD_INS " v2, (%2)\n"
                     "vcfmul.vv v3, v1, v2\n"
                     VST_INS " v3, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(n)
                     : "a0", "v1", "v2", "v3", "memory");

    printf("vcfmul.vv Test (VLEN=%d, ELEN=%d, N_ELE=%d)\n", VLEN, ELEN, (int)N_ELE);
    int pass = 1;
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-2) {
            pass = 0;
        }
    }
    if (!pass) {
        for (int i = 0; i < N_ELE; i++) {
            printf("[%d] Got %f, Expected %f\n", i, (double)vd[i], (double)expected[i]);
        }
    }
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
