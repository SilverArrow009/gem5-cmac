#include <math.h>
#include <stdio.h>

int
main()
{
    double rs1 = 2.0;
    double vs3[8] __attribute__((aligned(64))) = {1.0, 2.0, 3.0, 4.0,
                                                  5.0, 6.0, 7.0, 8.0};
    double vs2[8] __attribute__((aligned(64))) = {1.0, 0.0, 2.0, 0.0,
                                                  3.0, 0.0, 4.0, 0.0};
    double vd[8];
    double expected[8];

    for (int i = 0; i < 8; i++) {
        vd[i] = vs3[i];
    }

    for (int i = 0; i < 4; i++) {
        double vd_r = vs3[i * 2], vd_i = vs3[i * 2 + 1];
        expected[i * 2] = vs2[i * 2] + vd_r * rs1;
        expected[i * 2 + 1] = vs2[i * 2 + 1] + vd_i * rs1;
    }

    __asm__ volatile("fld f1, (%1)\n"
                     "li a0, 4\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vlseg2e64.v v2, (%2)\n" // v2=s2_r, v3=s2_i
                     "vlseg2e64.v v4, (%3)\n" // v4=vd_r, v5=vd_i
                     "vfmadd.vf v4, f1, v2\n"
                     "vfmadd.vf v5, f1, v3\n"
                     "vsseg2e64.v v4, (%0)\n"
                     :
                     : "r"(vd), "r"(&rs1), "r"(vs2), "r"(vs3)
                     : "a0", "v2", "v3", "v4", "v5", "f1");

    printf("vmadd.vf (complex) Test:\n");
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
