#!/bin/bash

GEM5_BIN="./build/RISCV/gem5.opt"
# CONFIG="./configs/cvext/baseline.py"
CONFIG="./configs/cvext/ara_baseline.py"
KERNEL_DIR="./tests/test-progs/cvext/rvv-1.0"
OUT_BASE="m5out/baseline"
DEBUG_FLAGS="--debug-flags=Exec,MinorTrace"

mkdir -p $OUT_BASE

# List of kernels to run
KERNELS=("vfmul_vv" "vfmul_vf" "vfdiv_vv" "vfdiv_vf" "vfmadd_vv" "vfmadd_vf" "vfmacc_vv" "vfmacc_vf" "vfmsac_vv" "vfmsac_vf" "vfmaccpyl_vv" "vfmaccpyl_vf" "vfmaccpyu_vv" "vfmaccpyu_vf" "vfpy" "matrix_mult")
# KERNELS=("vfmul_vv")

for kernel in "${KERNELS[@]}"; do
    if [ -f "$KERNEL_DIR/$kernel" ]; then
        echo "------------------------------------------------"
        echo "Running $kernel on Baseline Model..."
        $GEM5_BIN -re --outdir="$OUT_BASE/$kernel" $CONFIG "$KERNEL_DIR/$kernel"
    else
        echo "Warning: Kernel $kernel not found in $KERNEL_DIR"
    fi
done
