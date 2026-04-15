#include <math.h>
#include <stdio.h>

int
main()
{
    double vs2[8] __attribute__((aligned(64))) = {1.0, 2.0, 3.0, 4.0,
                                                  5.0, 6.0, 7.0, 8.0};
    double vd_pyl[8] __attribute__((aligned(64))) = {0};
    double vd_pyu[8] __attribute__((aligned(64))) = {0};
    double expected_pyl[8], expected_pyu[8];

    for (int i = 0; i < 4; i++) {
        double real = vs2[i * 2], imag = vs2[i * 2 + 1];
        // pyl: swap and imag = -real => (imag, -real)
        expected_pyl[i * 2] = imag;
        expected_pyl[i * 2 + 1] = -real;
        // pyu: swap and real = -imag => (-imag, real)
        expected_pyu[i * 2] = -imag;
        expected_pyu[i * 2 + 1] = real;
    }

    // PYL: (re, im) -> (im, -re)
    __asm__ volatile("li a0, 4\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vlseg2e64.v v10, (%1)\n" // v10=re, v11=im
                     "vfneg.v v12, v10\n"       // v12=-re
                     "vsseg2e64.v v11, (%0)\n"
                     :
                     : "r"(vd_pyl), "r"(vs2)
                     : "a0", "v10", "v11", "v12", "memory");

    // PYU: (re, im) -> (-im, re)
    __asm__ volatile("li a0, 4\n"
                     "vsetvli a0, a0, e64, m1\n"
                     "vlseg2e64.v v10, (%1)\n" // v10=re, v11=im
                     "vfneg.v v12, v11\n"       // v12=-im
                     "vmv.v.v v13, v10\n"       // v13=re
                     "vsseg2e64.v v12, (%0)\n" // store (v12, v13) = (-im, re)
                     :
                     : "r"(vd_pyu), "r"(vs2)
                     : "a0", "v10", "v11", "v12", "v13", "memory");
    int pass_pyl = 1, pass_pyu = 1;
    for (int i = 0; i < 8; i++) {
        if (fabs(vd_pyl[i] - expected_pyl[i]) > 1e-6) pass_pyl = 0;
        if (fabs(vd_pyu[i] - expected_pyu[i]) > 1e-6) pass_pyu = 0;
    }

    printf("vpy Test (Pauli-Y swap using standard RVV):\n");
    printf("Expected PYL: ");
    for (int i = 0; i < 8; i++) printf("%.2f ", expected_pyl[i]);
    printf("\nGot PYL:      ");
    for (int i = 0; i < 8; i++) printf("%.2f ", vd_pyl[i]);
    printf("\nExpected PYU: ");
    for (int i = 0; i < 8; i++) printf("%.2f ", expected_pyu[i]);
    printf("\nGot PYU:      ");
    for (int i = 0; i < 8; i++) printf("%.2f ", vd_pyu[i]);
    printf("\n");
    printf("Result PYL: %s\n", pass_pyl ? "PASS" : "FAIL");
    printf("Result PYU: %s\n", pass_pyu ? "PASS" : "FAIL");

    return (pass_pyl && pass_pyu) ? 0 : 1;
}
