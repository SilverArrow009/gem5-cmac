#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../common.h"

int main() {
    printf("vfdiv.vv (complex) Test | VLEN=%d, ELEN=%d\n", VLEN, ELEN);
    T vs1[N_ELE] __attribute__((aligned(64)));
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64))) = {0};
    T expected[N_ELE];

    for (int i = 0; i < N_ELE / 2; i++) {
        vs1[2 * i] = (T)i + 2.0;     // c
        vs1[2 * i + 1] = (T)i + 1.5; // d
        vs2[2 * i] = (T)i + 5.0;     // a
        vs2[2 * i + 1] = (T)i + 4.0; // b

        T a = vs2[2 * i], b = vs2[2 * i + 1];
        T c = vs1[2 * i], d = vs1[2 * i + 1];
        T den = c * c + d * d;
        expected[2 * i] = (a * c + b * d) / den;
        expected[2 * i + 1] = (b * c - a * d) / den;
    }

    __asm__ volatile(
        "li a0, %3\n"
        "vsetvli t0, a0, " VSET_E ", m1, ta, ma\n"
        VLD2_INS " v2, (%1)\n"  // v2=a, v3=b
        VLD2_INS " v4, (%2)\n"  // v4=c, v5=d
        "vfmul.vv v6, v4, v4\n" // c*c
        "vfmacc.vv v6, v5, v5\n" // c*c + d*d
        "vfmul.vv v8, v2, v4\n" // a*c
        "vfmacc.vv v8, v3, v5\n" // a*c + b*d
        "vfdiv.vv v0, v8, v6\n" // (a*c + b*d) / den
        "vfmul.vv v9, v3, v4\n" // b*c
        "vfnmsac.vv v9, v2, v5\n" // b*c - a*d
        "vfdiv.vv v1, v9, v6\n" // (b*c - a*d) / den
        VST2_INS " v0, (%0)\n"
        :
        : "r"(vd), "r"(vs2), "r"(vs1), "i"(N_ELE / 2)
        : "t0", "a0", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v8", "v9"
    );

    int pass = 1;
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-3) {
            pass = 0;
            break;
        }
    }

    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    if (!pass) {
        for (int i = 0; i < N_ELE / 2; i++) {
            printf("Pair %d: Expected (%.4f, %.4f), Got (%.4f, %.4f)\n",
                   i, (double)expected[2*i], (double)expected[2*i+1],
                   (double)vd[2*i], (double)vd[2*i+1]);
        }
    }

    return pass ? 0 : 1;
}
