#include <math.h>
#include <stdio.h>

int
main()
{
    double vs2[8] __attribute__((aligned(64))) = {1.0, 2.0, 3.0, 4.0,
                                                  5.0, 6.0, 7.0, 8.0};
    double vd_pyl[8] __attribute__((aligned(64)));
    double vd_pyu[8] __attribute__((aligned(64)));
    double expected_pyl[8], expected_pyu[8];

    for (int i = 0; i < 4; i++) {
        double real = vs2[i * 2], imag = vs2[i * 2 + 1];
        expected_pyl[i * 2] = imag;
        expected_pyl[i * 2 + 1] = -real;
        expected_pyu[i * 2] = -imag;
        expected_pyu[i * 2 + 1] = real;
    }

    __asm__ volatile("li a0, 8\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vle64.v v1, (%2)\n"
                     "vcfpyl.v v2, v1\n"
                     "vse64.v v2, (%0)\n"
                     "vcfpyu.v v3, v1\n"
                     "vse64.v v3, (%1)\n"
                     :
                     : "r"(vd_pyl), "r"(vd_pyu), "r"(vs2)
                     : "a0", "v1", "v2", "v3", "memory");

    printf("vcfpyl.v and vcfpyu.v Test:\n");
    printf("Input vs2:    ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", vs2[i]);
    }
    printf("\n");
    printf("vcfpyl Expected: ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", expected_pyl[i]);
    }
    printf("\n");
    printf("vcfpyl Got:      ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", vd_pyl[i]);
    }
    printf("\n");
    printf("vcfpyu Expected: ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", expected_pyu[i]);
    }
    printf("\n");
    printf("vcfpyu Got:      ");
    for (int i = 0; i < 8; i++) {
        printf("%.2f ", vd_pyu[i]);
    }
    printf("\n");

    int pass = 1;
    for (int i = 0; i < 8; i++) {
        if (fabs(vd_pyl[i] - expected_pyl[i]) > 1e-6) {
            pass = 0;
        }
        if (fabs(vd_pyu[i] - expected_pyu[i]) > 1e-6) {
            pass = 0;
        }
    }
    printf("Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
