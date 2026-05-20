CC = riscv64-unknown-elf-gcc
VLEN ?= 512
ELEN ?= 32
CFLAGS = -march=rv64gcv_zfh -mabi=lp64d -DVLEN=$(VLEN) -DELEN=$(ELEN)
BINS = vfmul_vv vfmul_vf vfdiv_vv vfdiv_vf vfmadd_vv vfmadd_vf vfmacc_vv vfmacc_vf \
			 vfmsac_vv vfmsac_vf vfmaccpyl_vv vfmaccpyl_vf vfmaccpyu_vv vfmaccpyu_vf vfpy \
			 matrix_mult

.PHONY: all clean

%: %.c
	$(CC) $(CFLAGS) $< -o $@

all: ${BINS}

clean:
	rm -rf ${BINS}
