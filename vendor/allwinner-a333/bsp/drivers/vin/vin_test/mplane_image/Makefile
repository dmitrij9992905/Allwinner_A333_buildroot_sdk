# SPDX-License-Identifier: GPL-2.0-or-later
# Makefile for the video capture/playback device drivers.
# Copyright 2010 Steven King <sfking@fdwdc.com>
#Copyright(c) 2022-2027 Allwinnertech Co., Ltd
# Each configuration option enables a list of files.
#longan system
#CC := ../../../../../out/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
#tina system
#CC:= not support
# andes kernel
#CC := ../../../../../out/toolchain/nds32le-linux-glibc-v5d/bin/riscv32-unknown-unknown-linux-gcc
# linux-6.6 andes kernel
#CC := ../../../../../out/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712/bin/riscv64-unknown-linux-gnu-gcc
##LD := ../../../../../out/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712/bin/riscv64-unknown-linux-gnu-ld
##AR := ../../../../../out/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712/bin/riscv64-unknown-linux-gnu-ar
#ARCH_DIR := ../../../../../kernel/linux-6.6-xuantie/arch/arm64/include/
#MARCH := rv32imafdcv_zicbom_zicbop_zicboz_zicond_zihintntl_zihintpause_zawrs_zfa_zfbfmin_zfh_zba_zbb_zbc_zbs_zvfbfmin_zvfbfwma_svinval_svnapot_svpbmt_xtheadc_xtheadvdot
#MTUNE := c907
#MABI := ilp32
##LD += -m elf32lriscv
#CFLAGS := -I $(ARCH_DIR) -march=$(MARCH) -mabi=$(MABI) -mtune=$(MTUNE)

#longan system
CC := ../../../../../out/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc
ARCH_DIR := ../../../../../kernel/linux-5.15/arch/arm64/include/
CFLAGS := -I $(ARCH_DIR)
TARGET := csi_test_mplane

.PHONY: all clean

all: $(TARGET)

csi_test_mplane: csi_test_mplane.c
	$(CC) $(CFLAGS) -static  $^  -o  $@  -lpthread -lc -lgcc_eh

	#mv csi_test_mplane ~/to_server/camera_test/

clean:
	rm -rf $(TARGET)
