# condition config

# arch

# kernel
ifeq ($(filter-out %6.6,$(LICHEE_KERN_VER)),)
	LICHEE_USE_INDEPENDENT_BSP := true
	ifeq ($(LICHEE_PLATFORM),linux)
		LICHEE_KERN_DEFCONF:=bsp_defconfig
		ifeq ($(LICHEE_LINUX_DEV),dragonboard)
			LICHEE_KERN_DEFCONF:=dragonboard_defconfig
		endif
		ifeq ($(LICHEE_LINUX_DEV),dragonabts)
			LICHEE_KERN_DEFCONF:=dragonabts_defconfig
		endif
	endif
endif

ifeq ($(LICHEE_PLATFORM),android)
	ifeq ($(LICHEE_KERN_DEFCONF),)
		LICHEE_KERN_DEFCONF := android15_arm64_defconfig
	endif
	ifeq ($(ANDROID_CLANG_PATH),)
		ANDROID_CLANG_PATH  := prebuilts/clang/host/linux-x86/clang-r510928/bin
	endif
	ifeq ($(filter-out %6.6,$(LICHEE_KERN_VER)),)
		ANDROID_GKI_VERSION := android15-6.6
	endif
	LICHEE_PACK_HOOK := build/hook/pack/hook.sh
endif
