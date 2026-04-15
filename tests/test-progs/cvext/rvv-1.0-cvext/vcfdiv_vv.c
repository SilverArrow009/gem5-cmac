#include <math.h>
#include <stdio.h>

int
main()
{
    double vs1[8] __attribute__((aligned(64))) = {4.0,  2.0,  6.0,  8.0,
                                                  10.0, 12.0, 14.0, 16.0};
    double vs2[8] __attribute__((aligned(64))) = {1.0, 1.0, 2.0, 2.0,
                                                  3.0, 3.0, 4.0, 4.0};
    double vd[8] __attribute__((aligned(64))) = {0};
    double expected[8];

    for (int i = 0; i < 4; i++) {
        double a = vs1[i * 2], b = vs1[i * 2 + 1];
        double c = vs2[i * 2], d = vs2[i * 2 + 1];
        double denom = c * c + d * d;
        expected[i * 2] = (a * c + b * d) / denom;
        expected[i * 2 + 1] = (b * c - a * d) / denom;
    }

    __asm__ volatile("li a0, 8\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vle64.v v1, (%1)\n"
                     "vle64.v v2, (%2)\n"
                     "vcfdiv.vv v3, v1, v2\n"
                     "vse64.v v3, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2)
                     : "a0", "v1", "v2", "v3", "memory");

    printf("vcfdiv.vv Test:\n");
    printf("Input vs1: ");
    for (int i = 0; i < 8; i++) {
        printf("%.4f ", vs1[i]);
    }
    printf("\n");
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
