#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../common.h"

int main() {
    printf("vfmaccpyu.vf (complex) Test | VLEN=%d, ELEN=%d\n", VLEN, ELEN);
    T fs1 = 1.5;
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd[N_ELE] __attribute__((aligned(64)));
    T expected[N_ELE];

    for (int i = 0; i < N_ELE / 2; i++) {
        vs2[2 * i] = (T)i + 2.0;     // re2
        vs2[2 * i + 1] = (T)1.0;     // im2
        vd[2 * i] = (T)i + 3.0;      // re_d
        vd[2 * i + 1] = (T)1.5;      // im_d

        T re2 = vs2[2 * i], im2 = vs2[2 * i + 1];
        T red = vd[2 * i], imd = vd[2 * i + 1];

        T re_acc = fs1 * re2 + red;
        T im_acc = fs1 * im2 + imd;

        expected[2 * i] = -im_acc;
        expected[2 * i + 1] = re_acc;
    }

    __asm__ volatile(
        LD_INS " f1, 0(%1)\n"
        "li a0, %2\n"
        "vsetvli t0, a0, " VSET_E ", m1, ta, ma\n"
        VLD2_INS " v0, (%0)\n"  // vd: v0=re_d, v1=im_d
        VLD2_INS " v2, (%3)\n"  // vs2: v2=re2, v3=im2
        "vfmacc.vf v0, f1, v2\n"  // v0 = re_f*re2 + re_d
        "vfmacc.vf v1, f1, v3\n"  // v1 = re_f*im2 + im_d
        "vmv.v.v v6, v0\n"        // v6 = re_acc
        "vfneg.v v0, v1\n"        // v0 = -im_acc
        "vmv.v.v v1, v6\n"        // v1 = re_acc
        VST2_INS " v0, (%0)\n"
        :
        : "r"(vd), "r"(&fs1), "i"(N_ELE / 2), "r"(vs2)
        : "t0", "a0", "v0", "v1", "v2", "v3", "v6", "f1", "f2"
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
