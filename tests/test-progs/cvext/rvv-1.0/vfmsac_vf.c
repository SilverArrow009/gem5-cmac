#include <math.h>
#include <stdio.h>

int
main()
{
    double rs1 = 2.0;
    double vs3[8] __attribute__((aligned(64))) = {10.0, 20.0, 30.0, 40.0,
                                                  50.0, 60.0, 70.0, 80.0};
    double vs2[8] __attribute__((aligned(64))) = {3.0, 4.0, 5.0, 6.0,
                                                  7.0, 8.0, 9.0, 10.0};
    double vd[8] __attribute__((aligned(64)));
    double expected[8] __attribute__((aligned(64)));

    for (int i = 0; i < 8; i++) {
        vd[i] = vs3[i];
    }

    for (int i = 0; i < 4; i++) {
        double d_r = vs3[i * 2], d_i = vs3[i * 2 + 1];
        double s2_r = vs2[i * 2], s2_i = vs2[i * 2 + 1];
        expected[i * 2] = rs1 * s2_r - d_r;
        expected[i * 2 + 1] = rs1 * s2_i - d_i;
    }

    __asm__ volatile("fld f1, (%1)\n"
                     "li a0, 8\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vle64.v v3, (%3)\n"
                     "vle64.v v2, (%2)\n"
                     "vfmsac.vf v3, f1, v2\n"
                     "vse64.v v3, (%0)\n"
                     :
                     : "r"(vd), "r"(&rs1), "r"(vs2), "r"(vs3)
                     : "a0", "f1", "v2", "v3", "memory");

    printf("vmsac.vf (complex) Test:\n");
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
