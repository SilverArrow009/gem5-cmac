#ifdef DEBUG
#include <stdio.h>
#include <math.h>
#endif
#include "../common.h"

int
main()
{
    T vs1[N_ELE] __attribute__((aligned(64)));
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64)));
    T expected[N_ELE];

    for (int i = 0; i < N_ELE / 2; i++) {
        vs1[2 * i] = (T)i + 1.0;     // re1
        vs1[2 * i + 1] = (T)0.5;     // im1
        vs2[2 * i] = (T)i + 2.0;     // re2
        vs2[2 * i + 1] = (T)1.0;     // im2
        vd[2 * i] = (T)i + 3.0;      // re_d
        vd[2 * i + 1] = (T)1.5;      // im_d

        T s1_r = vs1[i * 2], s1_i = vs1[i * 2 + 1];
        T s2_r = vs2[i * 2], s2_i = vs2[i * 2 + 1];
        T d_r  = vd[i * 2], d_i  = vd[i * 2 + 1];
        // vd = vs1 * vs2 + vd
        expected[i * 2] = (s1_r * s2_r - s1_i * s2_i + d_r);
        expected[i * 2 + 1] = (s1_r * s2_i + s1_i * s2_r + d_i);
    }

    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     VLD_INS " v1, (%1)\n"
                     VLD_INS " v2, (%2)\n"
                     VLD_INS " v3, (%0)\n"
                     "vcfmacc.vv v3, v1, v2\n"
                     VST_INS " v3, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(N_ELE)
                     : "a0", "v1", "v2", "v3", "memory");

    int pass = 1;
#ifdef DEBUG
    printf("vcfmacc.vv Test (VLEN=%d, ELEN=%d):\n", VLEN, ELEN);
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
