#include <math.h>
#include <stdio.h>

int
main()
{
    double rs1 = 2.0;
    double vs3[8] __attribute__((aligned(64))) = {1.0, 2.0, 3.0, 4.0,
                                                  5.0, 6.0, 7.0, 8.0};
    double vs2[8] __attribute__((aligned(64))) = {3.0, 4.0, 5.0, 6.0,
                                                  7.0, 8.0, 9.0, 10.0};
    double vd[8];
    double expected[8];

    for (int i = 0; i < 8; i++) {
        vd[i] = vs3[i];
    }

    for (int i = 0; i < 4; i++) {
        double d_r = vs3[i * 2], d_i = vs3[i * 2 + 1];
        double s2_r = vs2[i * 2], s2_i = vs2[i * 2 + 1];
        double res_r = d_r + rs1 * s2_r;
        double res_i = d_i + rs1 * s2_i;
        expected[i * 2] = -res_i;
        expected[i * 2 + 1] = res_r;
    }

    __asm__ volatile("fld f1, (%1)\n"
                     "li a0, 4\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vlseg2e64.v v2, (%2)\n" // v2=s2_r, v3=s2_i
                     "vlseg2e64.v v4, (%3)\n" // v4=d_r,  v5=d_i
                     "vfmacc.vf v4, f1, v2\n" // v4 = rs1*s2_r + d_r = res_r
                     "vfmacc.vf v5, f1, v3\n" // v5 = rs1*s2_i + d_i = res_i
                     "vfneg.v v6, v5\n"       // v6 = -res_i
                     "vmv.v.v v7, v4\n"       // v7 = res_r
                     "vsseg2e64.v v6, (%0)\n" // store (v6, v7) = (-res_i, res_r)
                     :
                     : "r"(vd), "r"(&rs1), "r"(vs2), "r"(vs3)
                     : "a0", "f1", "v2", "v3", "v4", "v5", "v6", "v7", "memory");

    printf("vmaccpyu.vf (complex) Test:\n");
    printf("Input rs1: %.2f\n", rs1);
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
