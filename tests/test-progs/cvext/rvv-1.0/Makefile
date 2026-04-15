CC = riscv64-unknown-elf-gcc
CFLAGS = -O2 -march=rv64gcv_zfh -mabi=lp64d

.PHONY: all

%: %.c
	$(CC) $(CFLAGS) $< -o $@

all: vfmul_vv vfmul_vf \
     vfdiv_vv vfdiv_vf \
     vfmadd_vv vfmadd_vf \
     vfmacc_vv vfmacc_vf \
     vfmsac_vv vfmsac_vf \
     vfmaccpyl_vv vfmaccpyl_vf \
     vfmaccpyu_vv vfmaccpyu_vf \
     vfpy \
     matrix_mult

clean: vfmul_vv vfmul_vf \
     vfdiv_vv vfdiv_vf \
     vfmadd_vv vfmadd_vf \
     vfmacc_vv vfmacc_vf \
     vfmsac_vv vfmsac_vf \
     vfmaccpyl_vv vfmaccpyl_vf \
     vfmaccpyu_vv vfmaccpyu_vf \
     vfpy \
     matrix_mult
	rm -rf $^
