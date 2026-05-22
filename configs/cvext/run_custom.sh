#!/bin/bash

GEM5_BIN="./build/RISCV/gem5.opt"
# Acceptable VLEN values are 128, 256, 512, 1024, 2048, 4096.
# Higher values do not lead to an error, but the model becomes inaccurate
VLEN=(512 1024 2048 4096)
# Acceptable values are 32, 64. Lower precision arithmetic WILL throw errors.
ELEN=(32 64)
CONFIG="./configs/cvext/ara_custom.py"
KERNEL_DIR="./tests/test-progs/cvext/rvv-1.0-cvext"
OUT_BASE="m5out/custom"

mkdir -p $OUT_BASE

# List of kernels to run
KERNELS=("vcfmul_vv" "vcfmul_vf" "vcfdiv_vv" "vcfdiv_vf" "vcfmadd_vv" "vcfmadd_vf" "vcfmacc_vv" "vcfmacc_vf" "vcfmsac_vv" "vcfmsac_vf" "vcfmaccpyl_vv" "vcfmaccpyl_vf" "vcfmaccpyu_vv" "vcfmaccpyu_vf" "vcfpy" "matrix_mult")

for vlen in "${VLEN[@]}"; do
    for elen in "${ELEN[@]}"; do
        make -C $KERNEL_DIR clean && make -C $KERNEL_DIR VLEN=$vlen ELEN=$elen all
        for kernel in "${KERNELS[@]}"; do
            if [ -f "$KERNEL_DIR/$kernel" ]; then
                echo "------------------------------------------------"
                echo "Running $kernel with VLEN=$vlen, ELEN=$elen on Custom Model..."
                $GEM5_BIN -re --outdir="$OUT_BASE/${kernel}_${vlen}_${elen}" $CONFIG --vlen=$vlen --elen=$elen "$KERNEL_DIR/$kernel"
            else
                echo "Warning: Kernel $kernel not found in $KERNEL_DIR"
            fi
        done
    done
done
