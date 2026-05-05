#!/bin/bash

# GEM5_BIN="./build/RISCV/gem5.debug"
GEM5_BIN="./build/RISCV/gem5.opt"
CONFIG="./configs/cvext/ara_custom.py"
# CONFIG="./configs/cvext/custom.py"
KERNEL_DIR="./tests/test-progs/cvext/rvv-1.0-cvext"
OUT_BASE="m5out/custom"

mkdir -p $OUT_BASE

# List of kernels to run
KERNELS=("vcfmul_vv" "vcfmul_vf" "vcfdiv_vv" "vcfdiv_vf" "vcfmadd_vv" "vcfmadd_vf" "vcfmacc_vv" "vcfmacc_vf" "vcfmsac_vv" "vcfmsac_vf" "vcfmaccpyl_vv" "vcfmaccpyl_vf" "vcfmaccpyu_vv" "vcfmaccpyu_vf" "vcfpy" "matrix_mult")

# KERNELS=("matrix_mult")

for kernel in "${KERNELS[@]}"; do
    if [ -f "$KERNEL_DIR/$kernel" ]; then
        echo "------------------------------------------------"
        echo "Running $kernel on Custom Model..."
        $GEM5_BIN -re --outdir="$OUT_BASE/$kernel" $CONFIG "$KERNEL_DIR/$kernel"
    else
        echo "Warning: Kernel $kernel not found in $KERNEL_DIR"
    fi
done
