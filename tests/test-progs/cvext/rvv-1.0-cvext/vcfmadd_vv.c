#include <stdio.h>
#include "../common.h"

int
main()
{
    T vs1[N_ELE] __attribute__((aligned(64)));
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64)));
    T vs3[N_ELE] __attribute__((aligned(64)));
    T expected[N_ELE];

    for (int i = 0; i < N_ELE; i++) {
        vs1[i] = (T)(i + 1.0);
        vs2[i] = (T)(i + 0.5);
        vs3[i] = (T)(i + 2.0);
        vd[i] = vs3[i];
    }

    for (int i = 0; i < N_ELE / 2; i++) {
        double s1_r = (double)vs1[i * 2], s1_i = (double)vs1[i * 2 + 1];
        double s2_r = (double)vs2[i * 2], s2_i = (double)vs2[i * 2 + 1];
        double vd_r = (double)vs3[i * 2], vd_i = (double)vs3[i * 2 + 1];
        // vd = vs1 * vd + vs2
        expected[i * 2] = (T)(s1_r * vd_r - s1_i * vd_i + s2_r);
        expected[i * 2 + 1] = (T)(s1_r * vd_i + s1_i * vd_r + s2_i);
    }

    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     VLD_INS " v1, (%1)\n"
                     VLD_INS " v2, (%2)\n"
                     VLD_INS " v3, (%0)\n"
                     "vcfmadd.vv v3, v1, v2\n"
                     VST_INS " v3, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(N_ELE)
                     : "a0", "v1", "v2", "v3", "memory");

    printf("vcfmadd.vv Test (VLEN=%d, ELEN=%d):\n", VLEN, ELEN);
    int pass = 1;
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-3) {
            pass = 0;
            break;
        }
    }
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
