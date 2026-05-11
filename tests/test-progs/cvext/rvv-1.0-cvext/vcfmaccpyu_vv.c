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
        vs1[i] = (T)(i + 2.0);
        vs2[i] = (T)((i % 2 == 0) ? (i / 2 + 1.0) : 0.0);
        vd[i] = (T)((i % 2 == 0) ? (i / 2 + 1.0) : 1.0);
    }

    for (int i = 0; i < N_ELE / 2; i++) {
        double s1_r = (double)vs1[i * 2], s1_i = (double)vs1[i * 2 + 1];
        double s2_r = (double)vs2[i * 2], s2_i = (double)vs2[i * 2 + 1];
        double d_r = (double)vd[i * 2], d_i = (double)vd[i * 2 + 1];
        double res_r = d_r + (s1_r * s2_r - s1_i * s2_i);
        double res_i = d_i + (s1_r * s2_i + s1_i * s2_r);
        // vcfpyu(res) = -res_i + res_r*i
        expected[i * 2] = (T)(-res_i);
        expected[i * 2 + 1] = (T)res_r;
    }

    int n = N_ELE;
    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     VLD_INS " v1, (%1)\n"
                     VLD_INS " v2, (%2)\n"
                     VLD_INS " v3, (%0)\n"
                     "vcfmaccpyu.vv v3, v1, v2\n"
                     VST_INS " v3, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(n)
                     : "a0", "v1", "v2", "v3", "memory");

    printf("vcfmaccpyu.vv Test (VLEN=%d, ELEN=%d, N_ELE=%d)\n", VLEN, ELEN, (int)N_ELE);
    int pass = 1;
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-3) {
            pass = 0;
            break;
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
