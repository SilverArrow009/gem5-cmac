#ifdef DEBUG
#include <stdio.h>
#endif
#include "../common.h"

int
main()
{
    T rs1 = (T)2.0;
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64)));
    T expected[N_ELE];

    for (int i = 0; i < N_ELE; i++) {
        vs2[i] = (T)(i + 0.5);
        vd[i] = (T)(i + 1.0);
    }

    for (int i = 0; i < N_ELE / 2; i++) {
        double s2_r = (double)vs2[i * 2], s2_i = (double)vs2[i * 2 + 1];
        double d_r = (double)vd[i * 2], d_i = (double)vd[i * 2 + 1];
        // vcfmsac.vf vd, rs1, vs2 => vd = rs1 * vs2 - vd
        expected[i * 2] = (T)(rs1 * s2_r - d_r);
        expected[i * 2 + 1] = (T)(rs1 * s2_i - d_i);
    }

    int n = N_ELE;
    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     LD_INS " f0, (%1)\n"
                     VLD_INS " v2, (%2)\n"
                     VLD_INS " v3, (%0)\n"
                     "vcfmsac.vf v3, f0, v2\n"
                     VST_INS " v3, (%0)\n"
                     :
                     : "r"(vd), "r"(&rs1), "r"(vs2), "r"(n), "i"(sizeof(T))
                     : "a0", "f0", "f1", "v2", "v3", "memory");

    int pass = 1;
#ifdef DEBUG
    printf("vcfmsac.vf Test (VLEN=%d, ELEN=%d, N_ELE=%d)\n", VLEN, ELEN, (int)N_ELE);
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
#endif
    return pass ? 0 : 1;
}
