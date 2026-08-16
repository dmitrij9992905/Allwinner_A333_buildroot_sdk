# SPDX-License-Identifier: GPL-2.0

g2d200_dir = g2d200
g2d200_obj = ${g2d200_dir}/g2d.o\
		  ${g2d200_dir}/g2d_debug.o ${g2d200_dir}/g2d_platform.o\
		  ${g2d200_dir}/g2d_rcq.o


ccflags-y += -I $(srctree)/drivers/hwspinlock

ifeq (${CONFIG_G2D_MIXER},y)
g2d200_obj += ${g2d200_dir}/g2d_mixer.o ${g2d200_dir}/g2d_ovl_v.o\
	   ${g2d200_dir}/g2d_ovl_u.o ${g2d200_dir}/g2d_bld.o\
	   ${g2d200_dir}/g2d_scal.o  ${g2d200_dir}/g2d_wb.o
endif

ifeq (${CONFIG_G2D_ROTATE},y)
g2d200_obj += ${g2d200_dir}/g2d_rotate.o
endif

ifneq (${CONFIG_G2D200},y)
g2d200_obj += ${g2d200_dir}/g2d_top.o
endif

ifeq (${CONFIG_G2D200},y)
g2d200_obj += ${g2d200_dir}/g2d_thread.o ${g2d200_dir}/g2d_core.o
endif
