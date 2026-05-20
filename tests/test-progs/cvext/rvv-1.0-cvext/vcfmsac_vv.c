#ifdef DEBUG
#include <stdio.h>
#endif
#include "../common.h"

int
main()
{
    T vs1[N_ELE] __attribute__((aligned(64)));
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vs3[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64)));
    T expected[N_ELE];

    for (int i = 0; i < N_ELE; i++) {
        vs1[i] = (T)(i + 2.0);
        vs2[i] = (T)(i + 1.5);
        vs3[i] = (T)(i + 1.0);
        vd[i] = vs3[i];
    }

    for (int i = 0; i < N_ELE / 2; i++) {
        double s1_r = (double)vs1[i * 2], s1_i = (double)vs1[i * 2 + 1];
        double s2_r = (double)vs2[i * 2], s2_i = (double)vs2[i * 2 + 1];
        double d_r = (double)vs3[i * 2], d_i = (double)vs3[i * 2 + 1];
        // vd = vs1 * vs2 - vd
        expected[i * 2] = (T)(s1_r * s2_r - s1_i * s2_i - d_r);
        expected[i * 2 + 1] = (T)(s1_r * s2_i + s1_i * s2_r - d_i);
    }

    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     VLD_INS " v1, (%1)\n"
                     VLD_INS " v2, (%2)\n"
                     VLD_INS " v3, (%0)\n"
                     "vcfmsac.vv v3, v1, v2\n"
                     VST_INS " v3, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(N_ELE)
                     : "a0", "v1", "v2", "v3", "memory");

    int pass = 1;
#ifdef DEBUG
    printf("vcfmsac.vv Test (VLEN=%d, ELEN=%d):\n", VLEN, ELEN);
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
