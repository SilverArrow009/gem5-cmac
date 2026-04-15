#include <stdio.h>

int
main()
{
    double rs1 = 2.0;
    double vs2[8] __attribute__((aligned(64))) = {1.0, 2.0, 3.0, 4.0,
                                                  5.0, 6.0, 7.0, 8.0};
    double vd[8] __attribute__((aligned(64))) = {0};
    double expected[8];

    for (int i = 0; i < 4; i++) {
        double c = vs2[i * 2], d = vs2[i * 2 + 1];
        expected[i * 2] = c * rs1;
        expected[i * 2 + 1] = d * rs1;
    }

    __asm__ volatile("fld f1, (%1)\n"
                     "li a0, 4\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vlseg2e64.v v2, (%2)\n" // v2=re, v3=im
                     "vfmul.vf v4, v2, f1\n"  // re * rs1
                     "vfmul.vf v5, v3, f1\n"  // im * rs1
                     "vsseg2e64.v v4, (%0)\n"
                     :
                     : "r"(vd), "r"(&rs1), "r"(vs2)
                     : "a0", "v2", "v3", "v4", "v5", "f1");

    printf("vfmul.vf (complex) Test:\n");
    printf("Input rs1: %.2f\n", rs1);
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
