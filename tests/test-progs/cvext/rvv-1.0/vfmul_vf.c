#ifdef DEBUG
#include <stdio.h>
#include <math.h>
#endif
#include "../common.h"

int main() {
    T fs1 = 2.0;
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64))) = {0};
    T expected[N_ELE];

    for (int i = 0; i < N_ELE / 2; i++) {
        vs2[2 * i] = (T)i + 1.0;
        vs2[2 * i + 1] = (T)i + 1.1;

        T a = vs2[2 * i];
        T b = vs2[2 * i + 1];

        expected[2 * i] = a * fs1;
        expected[2 * i + 1] = b * fs1;
    }

    __asm__ volatile(
        LD_INS " f1, (%1)\n"
        "li a0, %2\n"
        "vsetvli t0, a0, " VSET_E ", m1, ta, ma\n"
        VLD2_INS " v2, (%3)\n"
        "vfmul.vf v4, v2, f1\n"
        "vfmul.vf v5, v3, f1\n"
        VST2_INS " v4, (%0)\n"
        :
        : "r"(vd), "r"(&fs1), "i"(N_ELE / 2), "r"(vs2)
        : "t0", "a0", "v2", "v3", "v4", "v5", "f1"
    );

    int pass = 1;
#ifdef DEBUG
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-3) {
            pass = 0;
            break;
        }
    }
    printf("vfmul.vf (complex) Test | VLEN=%d, ELEN=%d\n", VLEN, ELEN);
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    if (!pass) {
        for (int i = 0; i < N_ELE / 2; i++) {
            printf("Pair %d: Expected (%.4f, %.4f), Got (%.4f, %.4f)\n",
                   i, (double)expected[2*i], (double)expected[2*i+1],
                   (double)vd[2*i], (double)vd[2*i+1]);
        }
    }
#endif

    return pass ? 0 : 1;
}
