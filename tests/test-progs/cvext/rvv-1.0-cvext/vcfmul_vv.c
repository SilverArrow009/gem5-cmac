#include <stdio.h>

int
main()
{
    double vs1[8] __attribute__((aligned(64))) = {1.0, 2.0, 3.0, 4.0,
                                                  5.0, 6.0, 7.0, 8.0};
    double vs2[8] __attribute__((aligned(64))) = {1.0, 1.0, 2.0, 2.0,
                                                  3.0, 3.0, 4.0, 4.0};
    double vd[8] __attribute__((aligned(64))) = {0};
    double expected[8];

    for (int i = 0; i < 4; i++) {
        double a = vs1[i * 2], b = vs1[i * 2 + 1];
        double c = vs2[i * 2], d = vs2[i * 2 + 1];
        expected[i * 2] = a * c - b * d;
        expected[i * 2 + 1] = a * d + b * c;
    }

    __asm__ volatile("li a0, 8\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vle64.v v1, (%1)\n"
                     "vle64.v v2, (%2)\n"
                     "vcfmul.vv v3, v1, v2\n"
                     "vse64.v v3, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2)
                     : "a0", "v1", "v2", "v3", "memory");

    printf("vcfmul.vv Test:\n");
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
        if (vd[i] != expected[i]) {
            pass = 0;
        }
    }
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
