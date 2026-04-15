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

    for (int i = 0; i < 8; i++) {
        vd[i] = vs3[i];
    }

    for (int i = 0; i < 4; i++) {
        double vd_r = vs3[i * 2], vd_i = vs3[i * 2 + 1];
        double s1_r = vs1[i * 2], s1_i = vs1[i * 2 + 1];
        double s2_r = vs2[i * 2], s2_i = vs2[i * 2 + 1];
        expected[i * 2] = s2_r + vd_r * s1_r - vd_i * s1_i;
        expected[i * 2 + 1] = s2_i + vd_r * s1_i + vd_i * s1_r;
    }

    __asm__ volatile("li a0, 4\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vlseg2e64.v v10, (%1)\n" // v10=s1_r, v11=s1_i
                     "vlseg2e64.v v12, (%2)\n" // v12=s2_r, v13=s2_i
                     "vlseg2e64.v v14, (%3)\n" // v14=vd_r, v15=vd_i
                     "vmv.v.v v16, v14\n"       // copy vd_r

                     // Real: s1_r*vd_r - s1_i*vd_i + s2_r
                     "vfmadd.vv v14, v10, v12\n"
                     "vfnmsac.vv v14, v11, v15\n"

                     // Imag: s1_r*vd_i + s1_i*vd_r + s2_i
                     "vfmadd.vv v15, v10, v13\n"
                     "vfmacc.vv v15, v11, v16\n"

                     "vsseg2e64.v v14, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2), "r"(vs3)
                     : "a0", "v10", "v11", "v12", "v13", "v14", "v15", "v16");

    printf("vmadd.vv (complex) Test:\n");
    printf("Input vs1: ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", vs1[i]);
    }
    printf("\n");
    printf("Input vs2: ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", vs2[i]);
    }
    printf("\n");
    printf("Input vd:  ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", vs3[i]);
    }
    printf("\n");
    printf("Expected:  ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", expected[i]);
    }
    printf("\n");
    printf("Got:       ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", vd[i]);
    }
    printf("\n");

    int pass = 1;
    for (int i = 0; i < 8; i++) {
        if (fabs(vd[i] - expected[i]) > 1e-6) {
            pass = 0;
        }
    }
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
