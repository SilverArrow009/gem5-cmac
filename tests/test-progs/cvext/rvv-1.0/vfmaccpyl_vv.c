#include <math.h>
#include <stdio.h>

int
main()
{
    double vs3[8] __attribute__((aligned(64))) = {1.0, 1.0, 2.0, 2.0,
                                                  3.0, 3.0, 4.0, 4.0};
    double vs1[8] __attribute__((aligned(64))) = {2.0, 3.0, 4.0, 5.0,
                                                  6.0, 7.0, 8.0, 9.0};
    double vs2[8] __attribute__((aligned(64))) = {1.0, 0.0, 2.0, 0.0,
                                                  3.0, 0.0, 4.0, 0.0};
    double vd[8];
    double expected[8];

    for (int i = 0; i < 4; i++) {
        double s1_r = vs1[i * 2], s1_i = vs1[i * 2 + 1];
        double s2_r = vs2[i * 2], s2_i = vs2[i * 2 + 1];
        double d_r = vs3[i * 2], d_i = vs3[i * 2 + 1];
        // Standard MACC: res = d + s1 * s2
        double res_r = d_r + (s1_r * s2_r - s1_i * s2_i);
        double res_i = d_i + (s1_r * s2_i + s1_i * s2_r);
        // PYL: (res_r, res_i) -> (res_i, -res_r)
        expected[i * 2] = res_i;
        expected[i * 2 + 1] = -res_r;
    }

    __asm__ volatile("li a0, 4\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vlseg2e64.v v10, (%1)\n" // v10=re1, v11=im1
                     "vlseg2e64.v v12, (%2)\n" // v12=re2, v13=im2
                     "vlseg2e64.v v14, (%3)\n" // v14=re_acc, v15=im_acc

                     // Complex MACC
                     // real_acc = real_acc + re1*re2 - im1*im2
                     "vfmacc.vv v14, v10, v12\n"
                     "vfnmsac.vv v14, v11, v13\n"
                     // imag_acc = imag_acc + re1*im2 + im1*re2
                     "vfmacc.vv v15, v10, v13\n"
                     "vfmacc.vv v15, v11, v12\n"

                     // PYL swap: (v14, v15) -> (v15, -v14)
                     "vfneg.v v16, v14\n"
                     "vsseg2e64.v v15, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(vs3)
                     : "a0", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "memory");

    printf("vmaccpyl.vv (complex) Test:\n");
    printf("Expected:  ");
    for (int i = 0; i < 8; i++) printf("%.2f ", expected[i]);
    printf("\nGot:       ");
    for (int i = 0; i < 8; i++) printf("%.2f ", vd[i]);
    printf("\n");

    int pass = 1;
    for (int i = 0; i < 8; i++) {
        if (fabs(vd[i] - expected[i]) > 1e-6) pass = 0;
    }
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
