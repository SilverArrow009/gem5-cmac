#ifdef DEBUG
#include <stdio.h>
#include <math.h>
#endif
#include "../common.h"

int main() {
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

        T re1 = vs1[2 * i], im1 = vs1[2 * i + 1];
        T re2 = vs2[2 * i], im2 = vs2[2 * i + 1];
        T red = vd[2 * i], imd = vd[2 * i + 1];

        T re_acc = re1 * re2 - im1 * im2 + red;
        T im_acc = re1 * im2 + im1 * re2 + imd;

        expected[2 * i] = im_acc;
        expected[2 * i + 1] = -re_acc;
    }

    __asm__ volatile(
        "li a0, %3\n"
        "vsetvli t0, a0, " VSET_E ", m1, ta, ma\n"
        VLD2_INS " v0, (%0)\n"  // vd: v0=re_d, v1=im_d
        VLD2_INS " v2, (%1)\n"  // vs1: v2=re1, v3=im1
        VLD2_INS " v4, (%2)\n"  // vs2: v4=re2, v5=im2
        "vfmacc.vv v0, v2, v4\n"  // v0 = re1*re2 + re_d
        "vfnmsac.vv v0, v3, v5\n" // v0 = re1*re2 + re_d - im1*im2
        "vfmacc.vv v1, v2, v5\n"  // v1 = re1*im2 + im_d
        "vfmacc.vv v1, v3, v4\n"  // v1 = re1*im2 + im_d + im1*re2
        "vmv.v.v v6, v0\n"        // v6 = re_acc
        "vmv.v.v v0, v1\n"        // v0 = im_acc
        "vfneg.v v1, v6\n"        // v1 = -re_acc
        VST2_INS " v0, (%0)\n"
        :
        : "r"(vd), "r"(vs1), "r"(vs2), "i"(N_ELE / 2)
        : "t0", "a0", "v0", "v1", "v2", "v3", "v4", "v5", "v6"
    );

    int pass = 1;
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd[i] - (double)expected[i]) > 1e-3) {
            pass = 0;
            break;
        }
    }

#ifdef DEBUG
    printf("vfmaccpyl.vv (complex) Test | VLEN=%d, ELEN=%d\n", VLEN, ELEN);
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
