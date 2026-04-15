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

    /* Standard RVV 1.0 for complex division:
     * (a+bi)/(c+di) = (ac+bd)/(c^2+d^2) + (bc-ad)i/(c^2+d^2)
     */
    __asm__ volatile("li a0, 4\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vlseg2e64.v v10, (%1)\n" // v10=re1, v11=im1
                     "vlseg2e64.v v12, (%2)\n" // v12=re2, v13=im2

                     // Denominator: re2*re2 + im2*im2
                     "vfmul.vv v14, v12, v12\n"
                     "vfmacc.vv v14, v13, v13\n"

                     // Real numerator: re1*re2 + im1*im2
                     "vfmul.vv v15, v10, v12\n"
                     "vfmacc.vv v15, v11, v13\n"

                     // Imag numerator: im1*re2 - re1*im2
                     "vfmul.vv v16, v11, v12\n"
                     "vfnmsac.vv v16, v10, v13\n"

                     // Divide by denominator
                     "vfdiv.vv v15, v15, v14\n"
                     "vfdiv.vv v16, v16, v14\n"

                     "vsseg2e64.v v15, (%0)\n"
                     :
                     : "r"(vd), "r"(vs1), "r"(vs2)
                     : "a0", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "memory");

    printf("vdiv.vv (complex) Test:\n");
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
