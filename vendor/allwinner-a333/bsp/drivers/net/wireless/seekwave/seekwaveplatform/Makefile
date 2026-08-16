#
#SeekWave Platform Drivers Makefile
#
#
PWD :=$(shell pwd)
CURFOLDER ?=$(pwd)

ccflags-y :=-Idrivers/net/wireless/seekwave/seekwaveplatform/sdio
ccflags-y +=-Ilongan/bsp/drivers/net/wireless/seekwave/seekwaveplatform/usb
ccflags-y +=-Ilongan/bsp/drivers/net/wireless/seekwave/seekwaveplatform/sdio



ifeq ($(CONFIG_SKW_BT),m)
        ccflags-y += -DCONFIG_BT_SEEKWAVE
endif

ifneq ($(skw_extra_flags),)
        ccflags-y += $(skw_extra_flags) -DSKW_EXT_INC
endif

ifeq ($(CONFIG_OF), y)
        #ccflags-y += -DCONFIG_NO_DTS
endif

ccflags-y += -I$(srctree)/drivers/net/wireless/seekwave
ccflags-y += -DCONFIG_SV6160_LITE_FPGA

obj-$(CONFIG_SKW_SDIOHAL) += skw_sdio.o
skw_sdio-y := ./sdio/skw_sdio_main.o
skw_sdio-y += ./sdio/skw_sdio_rx.o
skw_sdio-y += ./sdio/skw_sdio_debugfs.o
skw_sdio-y += ./sdio/skw_sdio_log.o
skw_sdio-y += ./skwutil/skw_user_com.o
skw_sdio-y += ./skwutil/skw_log_to_file.o
skw_sdio-y += ./skwutil/skw_boot.o

clean:
	@rm -rf *.o *.ko *.mod.c *.order *.a *.builtin .*.cmd .*.d

