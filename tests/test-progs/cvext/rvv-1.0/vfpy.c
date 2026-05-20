#ifdef DEBUG
#include <stdio.h>
#include <math.h>
#endif
#include "../common.h"

int main() {
#ifdef DEBUG
    printf("vfpy (complex) Test | VLEN=%d, ELEN=%d\n", VLEN, ELEN);
#endif
    T vs2[N_ELE] __attribute__((aligned(64)));
    T vd_pyl[N_ELE] __attribute__((aligned(64))) = {0};
    T vd_pyu[N_ELE] __attribute__((aligned(64))) = {0};
    T expected_pyl[N_ELE], expected_pyu[N_ELE];

    for (int i = 0; i < N_ELE / 2; i++) {
        vs2[2 * i] = (T)i + 1.0;
        vs2[2 * i + 1] = (T)i + 1.5;

        T real = vs2[2 * i], imag = vs2[2 * i + 1];
        // pyl: (re, im) -> (im, -re)
        expected_pyl[2 * i] = imag;
        expected_pyl[2 * i + 1] = -real;
        // pyu: (re, im) -> (-im, re)
        expected_pyu[2 * i] = -imag;
        expected_pyu[2 * i + 1] = real;
    }

    // PYL
    __asm__ volatile(
        "li a0, %2\n"
        "vsetvli t0, a0, " VSET_E ", m1, ta, ma\n"
        VLD2_INS " v10, (%1)\n" // v10=re, v11=im
        "vfneg.v v12, v10\n"    // v12=-re
        VST2_INS " v11, (%0)\n" // store (v11, v12) = (im, -re)
        :
        : "r"(vd_pyl), "r"(vs2), "i"(N_ELE / 2)
        : "t0", "a0", "v10", "v11", "v12", "memory"
    );

    // PYU
    __asm__ volatile(
        "li a0, %2\n"
        "vsetvli t0, a0, " VSET_E ", m1, ta, ma\n"
        VLD2_INS " v10, (%1)\n" // v10=re, v11=im
        "vfneg.v v12, v11\n"    // v12=-im
        "vmv.v.v v13, v10\n"    // v13=re
        VST2_INS " v12, (%0)\n" // store (v12, v13) = (-im, re)
        :
        : "r"(vd_pyu), "r"(vs2), "i"(N_ELE / 2)
        : "t0", "a0", "v10", "v11", "v12", "v13", "memory"
    );

    int pass_pyl = 1, pass_pyu = 1;
#ifdef DEBUG
    for (int i = 0; i < N_ELE; i++) {
        if (fabs((double)vd_pyl[i] - (double)expected_pyl[i]) > 1e-3) pass_pyl = 0;
        if (fabs((double)vd_pyu[i] - (double)expected_pyu[i]) > 1e-3) pass_pyu = 0;
    }

    printf("vfpy (complex) Test | VLEN=%d, ELEN=%d\n", VLEN, ELEN);
    printf("Result PYL: %s\n", pass_pyl ? "PASS" : "FAIL");
    printf("Result PYU: %s\n", pass_pyu ? "PASS" : "FAIL");
#endif
    return (pass_pyl && pass_pyu) ? 0 : 1;
}
