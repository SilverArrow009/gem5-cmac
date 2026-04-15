CC = riscv64-unknown-elf-gcc
CFLAGS = -O2 -march=rv64gcv_zfh -mabi=lp64d

.PHONY: all clean

%: %.c
	$(CC) $(CFLAGS) $< -o $@

all: vcfmul_vv vcfmul_vf \
      vcfdiv_vv vcfdiv_vf \
      vcfmadd_vv vcfmadd_vf \
      vcfmacc_vv vcfmacc_vf \
      vcfmsac_vv vcfmsac_vf \
      vcfmaccpyl_vv vcfmaccpyl_vf \
      vcfmaccpyu_vv vcfmaccpyu_vf \
      vcfpy \
      matrix_mult

clean: vcfmul_vv vcfmul_vf \
      vcfdiv_vv vcfdiv_vf \
      vcfmadd_vv vcfmadd_vf \
      vcfmacc_vv vcfmacc_vf \
      vcfmsac_vv vcfmsac_vf \
      vcfmaccpyl_vv vcfmaccpyl_vf \
      vcfmaccpyu_vv vcfmaccpyu_vf \
      vcfpy \
      matrix_mult
	rm -rf $^
