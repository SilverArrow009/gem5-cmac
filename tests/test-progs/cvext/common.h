#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <math.h>

#ifndef VLEN
#define VLEN 1024
#endif

#ifndef ELEN
#define ELEN 64
#endif

#define N_ELE (VLEN / ELEN)

#if ELEN == 64
typedef double T;
#define VSET_E "e64"
#define VLD_INS "vle64.v"
#define VST_INS "vse64.v"
#define LD_INS "fld"
#define FMV_F "vfmv.v.f"
#define VLD2_INS "vlseg2e64.v"
#define VST2_INS "vsseg2e64.v"
#elif ELEN == 32
typedef float T;
#define VSET_E "e32"
#define VLD_INS "vle32.v"
#define VST_INS "vse32.v"
#define LD_INS "flw"
#define FMV_F "vfmv.v.f"
#define VLD2_INS "vlseg2e32.v"
#define VST2_INS "vsseg2e32.v"
#elif ELEN < 16
#undef ELEN
#define ELEN 64
#endif

// These formats are not typically used in the context of quantum computing
// Keeping the raw 16 bit and 8 bit versions of the instructions if someone wants to fork
// it for ML research
//
// typedef _Float16 T;
// #define VSET_E "e16"
// #define VLD_INS "vle16.v"
// #define VST_INS "vse16.v"
// #define LD_INS "flh"
// #define FMV_F "vfmv.v.f"
// #define VLD2_INS "vlseg2e16.v"
// #define VST2_INS "vsseg2e16.v"
// #elif ELEN == 8
// typedef int8_t T;
// #define VSET_E "e8"
// #define VLD_INS "vle8.v"
// #define VST_INS "vse8.v"
// #define LD_INS "lb"
// #define FMV_F "vmv.v.x"
// #define VLD2_INS "vlseg2e8.v"
// #define VST2_INS "vsseg2e8.v"
// #endif

#endif
