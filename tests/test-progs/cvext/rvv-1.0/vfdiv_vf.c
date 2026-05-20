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

    T c = fs1;

    for (int i = 0; i < N_ELE / 2; i++) {
        vs2[2 * i] = (T)i + 5.0;     // a
        vs2[2 * i + 1] = (T)i + 4.0; // b

        T a = vs2[2 * i], b = vs2[2 * i + 1];
        expected[2 * i] = a / c;
        expected[2 * i + 1] = b / c;
    }

    __asm__ volatile(
        LD_INS " f1, (%1)\n"
        "li a0, %2\n"
        "vsetvli t0, a0, " VSET_E ", m1, ta, ma\n"
        VLD2_INS " v2, (%3)\n" // v2=a, v3=b
        "vfdiv.vf v0, v2, f1\n"
        "vfdiv.vf v1, v3, f1\n"
        VST2_INS " v0, (%0)\n"
        :
        : "r"(vd), "r"(&fs1), "i"(N_ELE / 2), "r"(vs2)
        : "t0", "a0", "v0", "v1", "v2", "v3", "v6", "v7", "f1", "f2", "f3"
    );

    int pass = 1;
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-3) {
            pass = 0;
            break;
        }
    }

#ifdef DEBUG
    printf("vfdiv.vf (complex) Test | VLEN=%d, ELEN=%d\n", VLEN, ELEN);
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
