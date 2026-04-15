#include <math.h>
#include <stdio.h>

int
main()
{
    double rs1 = 4.0;
    double vs2[8] __attribute__((aligned(64))) = {2.0, 0.0, 4.0, 0.0,
                                                  6.0, 0.0, 8.0, 0.0};
    double vd[8] __attribute__((aligned(64))) = {0};
    double expected[8];

    for (int i = 0; i < 4; i++) {
        double c = vs2[i * 2], d = vs2[i * 2 + 1];
        expected[i * 2] = c / rs1;
        expected[i * 2 + 1] = d / rs1;
    }

    __asm__ volatile("fld f1, (%1)\n"
                     "li a0, 8\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vle64.v v2, (%2)\n"
                     "vcfdiv.vf v3, v2, f1\n"
                     "vse64.v v3, (%0)\n"
                     :
                     : "r"(vd), "r"(&rs1), "r"(vs2)
                     : "a0", "f1", "v2", "v3", "memory");

    printf("vcfdiv.vf Test:\n");
    printf("Input rs1: %.2f\n", rs1);
    printf("Input vs2: ");
    for (int i = 0; i < 8; i++) {
        printf("%.4f ", vs2[i]);
    }
    printf("\n");
    printf("Expected:  ");
    for (int i = 0; i < 8; i++) {
        printf("%.4f ", expected[i]);
    }
    printf("\n");
    printf("Got:       ");
    for (int i = 0; i < 8; i++) {
        printf("%.4f ", vd[i]);
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
