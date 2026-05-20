#ifdef DEBUG
#include <stdio.h>
#endif
#include "../common.h"

int
main()
{
    T rs1 = (T)4.0;
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64)));
    T expected[N_ELE];

    for (int i = 0; i < N_ELE; i++) {
        vs2[i] = (T)(i + 2.0);
        vd[i] = (T)0.0;
    }

    for (int i = 0; i < N_ELE / 2; i++) {
        double c = (double)vs2[i * 2], d = (double)vs2[i * 2 + 1];
        double denom = rs1;
        expected[i * 2] = (T)((c) / denom);
        expected[i * 2 + 1] = (T)((d) / denom);
    }

    int n = N_ELE;
    __asm__ volatile("mv a0, %3\n"
                     "vsetvli a0, a0, " VSET_E ", m1\n"
                     LD_INS " f0, 0(%1)\n"
                     VLD_INS " v2, (%2)\n"
                     "vcfdiv.vf v3, v2, f0\n"
                     VST_INS " v3, (%0)\n"
                     :
                     : "r"(vd), "r"(&rs1), "r"(vs2), "r"(n), "i"(sizeof(T))
                     : "a0", "f0", "f1", "v2", "v3", "memory");

    int pass = 1;
#ifdef DEBUG
    printf("vcfdiv.vf Test (VLEN=%d, ELEN=%d, N_ELE=%d)\n", VLEN, ELEN, (int)N_ELE);
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
