#######################################################
# Filename     : fast_switch.sh
# Last modified: 2021-04-12 17:09
# Author       : jzzh
# Email        : jzzh@szbaijie.cn
# Company site : http://www.szbaijie.cn/index.php
# Description  :
#######################################################
#!/bin/sh

function build_usage()
{
    printf "Usage: linux build environment:
    mlongan             - default build all.
    mdts                - only build devicetree.
    mkernel             - only build kernel.
    menuconfig          - kernel menuconfig.
    mboot               - only build bootloader.
    mconfig             - only build config.
    pack                - pack firmware.
    release_customer    - custom firmware.
"
    printf "Usage: file jump environment:
    croot               - Changes directory to the top of the tree.
    cboot               - Changes directory to the bootloader.
    cdts                - Changes directory to the kernel devicetree.
    ckernel             - Changes directory to the kernel.
    cdrivers            - Changes directory to the kernel driver.
    cconfigs            - Changes directory to the board's all config.
    cconfig             - Changes directory to the board's config.
    cextra              - Changes directory to the product's extra.
    crootfs             - Changes directory to the product's rootfs.
    cdevice             - Changes directory to the product's dragonboard.
    cimg                - Changes directory to the pack firmware.
    cout                - Changes directory to the out.
    candroid            - Changes directory to the android.
    clinux              - Changes directory to the linux.
"
    return 0
}

function gettop
{
    local TOPFILE=build/szbaijie_env.sh
    if [ -n "$TARGET_TOP" -a -f "$TARGET_TOP/$TOPFILE" ] ; then
        # The following circumlocution ensures we remove symlinks from TOP.
        cd $TARGET_TOP; PWD= /bin/pwd
	 else
		if [ -f $TOPFILE ] ; then
            # The following circumlocution (repeated below as well) ensures
            # that we record the true directory name and not one that is
            # faked up with symlink names.
            PWD= /bin/pwd;
        else
            local here="${PWD}"
            while [ "${here}" != "/" ]; do
                if [ -f "${here}/${TOPFILE}" ]; then
                    (\cd ${here}; PWD= /bin/pwd)
                    break
                fi
                here="$(dirname ${here})"
            done
		fi
    fi
}

# == jump directory ==
function croot()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	\cd $T
}

function cbsp()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_KERNEL_VERSION/bsp
}

function cboot()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $T/$TARGET_UBOOT
}

function cdts()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_DTS
}

function ckernel
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_KERNEL_VERSION
}

function cdrivers
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_DRIVER
}

function cconfigs
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_CONFIG/
}

function cconfig
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_CONFIG/linux-6.6
}

function cextra
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_EXTRA
}

function crootfs
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_ROOTFS
}

function cextra_bj
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_DEVICE/baijie_extra
}

function crootfs_bj
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_DEVICE/baijie_system
}

function cdevice
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_DEVICE
}

function cimg
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_IMG
}

function cout
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $TARGET_OUT
}



function candroid
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $T/../
	. build/envsetup.sh
	lunch 15
}

function release_customer
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $T
	. build/envsetup.sh >/dev/null

	cconfig
	export CUSTOMER=`git branch  |grep "*" |cut -b3-`
	cd - >/dev/null

	local MAIN_BRANCH=`echo $CUSTOMER | cut -b 1-6`
	if [ $MAIN_BRANCH == "helper" ]; then
		local CUSTOMER=tmp
		local REL_DIR=/home/share_disk/${CUSTOMER}
		if [ ! -d $REL_DIR ]; then
			mkdir -p $REL_DIR
		fi
	else
		local REL_DIR=/home/share_disk/${CUSTOMER}
		if [ ! -d $REL_DIR ]; then
			mkdir -p $REL_DIR
		fi
	fi

	if [ "`cat $T/.buildconfig | grep -w LICHEE_PLATFORM | awk -F '=' '{ print $2}'`" = "android" ]; then
		local PLATFORM=${LICHEE_PLATFORM}10
	else
		local PLATFORM=$LICHEE_LINUX_DEV
	fi
	local TIME_STAMP=`date +%Y%m%d_%H%M`
	if [ -n "$1" ]; then
		local INPUTPARA=_$1
	else
		local INPUTPARA=
	fi
	local FIRMWARE_NAME=${LICHEE_IC}_${PLATFORM}_${LICHEE_BOARD}_uart0.img
	local REL_NAME=${FIRMWARE_NAME}${INPUTPARA}_${TIME_STAMP}.gz

	cd $LICHEE_OUT_DIR
	echo "Compressing firmware by pigz..."
	time pigz -k -f ${FIRMWARE_NAME}
	echo ""
	echo -e '\033[0;31;1m'
	echo "Move to customer download directory and please ignore the following permission message"
	echo -e '\033[0m'
	mv ${FIRMWARE_NAME}.gz ${REL_DIR}/${REL_NAME}
	cd - >/dev/null

	echo -e '\033[0;31;1m'
	echo "http://down.szbaijie.com:8088/${CUSTOMER}/${REL_NAME}"
	echo -e '\033[0m'
}

function clinux()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $T/longan
	. build/szbaijie_env.sh
}

# == build project ==
function mlongan
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	if [ ! "$1" = "distclean" ] && [ ! "$1" = "clean" ] && [ ! "$1" = "debug" ]; then
		if [ "`cat $T/.buildconfig | grep -w LICHEE_PLATFORM | awk -F '=' '{ print $2}'`" = "android" ]; then
			\cd $T && ./build.sh
		else
			\cd $T && ./build.sh && ./build.sh pack
		fi
	else
		if [ "$1" = "d" ]; then
			\cd $T && ./build.sh && ./build.sh pack_debug
		else
			echo "=== longan .config $1 ==="
			\cd $T && ./build.sh $1
		fi
	fi

	cd -
}

function mdts
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	if [ ! "$1" = "distclean" ] && [ ! "$1" = "clean" ]; then
		\cd $T && ./build.sh dts
	else
		\cd $T && ./build.sh $1
	fi


	cd -
}

function mkernel
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	if [ ! "$1" = "distclean" ] && [ ! "$1" = "clean" ]; then
		\cd $T && ./build.sh kernel $1
	else
		\cd $T && ./build.sh $1
	fi


	cd -
}

function menuconfig
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	if [ $1 = "save" ] || [ $1 = "load" ]; then
		\cd $T && ./build.sh $1config
	else
		\cd $T && ./build.sh menuconfig
	fi

	cd -
}

function mboot
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	if [ -f $T/$TARGET_UBOOT/.config ]; then
		if [ ! "$1" = "distclean" ] && [ ! "$1" = "clean" ]; then
			rm $T/$TARGET_UBOOT/drivers/video/sunxi/disp2/disp/lcd/*.o
			\cd $T/$TARGET_UBOOT && make -j
		else
			\cd $T/$TARGET_UBOOT && make $1
		fi
	else
		if [ "$1" = "distclean" ] || [ "$1" = "clean" ]; then
			\cd $T/$TARGET_UBOOT && make $1
		else
			\cd $T/$TARGET_UBOOT && make TARGET_DEFCONF && make -j
		fi
	fi

	cd -
}

function mconfig
{
	local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	echo "=== reconfig .buildconfig ==="
	if [ -f "$T/.buildconfig" ]; then
		\cd $T/  && ./build.sh config
		#rm $T/.buildconfig && \cd $T/  && ./build.sh config
	else
		\cd $T/  && ./build.sh config
	fi

	cd -
}

function pack
{
	local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	echo "=== pack ==="
	\cd $T/  && ./build.sh dts && ./build.sh pack

	cd -
}

function main()
{
	local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	if [ -f "$T/.buildconfig" ] && [ -n "`cat $T/.buildconfig | grep LICHEE_LINUX_DEV | awk -F '=' '{ print $2}'`" ] \
		&& [ "`cat $T/.buildconfig | grep -w LICHEE_PLATFORM | awk -F '=' '{ print $2}'`" = "linux" ] \
		|| [ "`cat $T/.buildconfig | grep -w LICHEE_PLATFORM | awk -F '=' '{ print $2}'`" = "android" ]; then
		export TARGET_TOP="$(PWD= /bin/pwd)"

		TARGET_KERNEL_VERSION=`cat $T/.buildconfig | grep LICHEE_KERN_DIR | awk -F '=' '{ print $2}'`
		TARGET_BSP=`cat $T/.buildconfig | grep LICHEE_BSP_DIR | awk -F '=' '{ print $2}'`
		TARGET_DEVICE=`cat $T/.buildconfig | grep LICHEE_DRAGONBAORD_DIR | awk -F '=' '{ print $2}'`
		TARGET_OUT=`cat $T/.buildconfig | grep LICHEE_PLAT_OUT | awk -F '=' '{ print $2}'`
		TARGET_CONFIG=`cat $T/.buildconfig | grep LICHEE_BOARD_CONFIG_DIR | awk -F '=' '{ print $2}'`
		TARGET_IMG=`cat $T/.buildconfig | grep LICHEE_OUT_DIR | awk -F '=' '{ print $2}'`
		TARGET_DEFCONF=`cat $T/.buildconfig | grep LICHEE_BRANDY_DEFCONF | awk -F '=' '{ print $2}'`

		TARGET_UBOOT="brandy/brandy-2.0/u-boot-2018"
		TARGET_DRIVER="$TARGET_KERNEL_VERSION/drivers/"
		TARGET_DTS="$TARGET_BSP/configs/linux-6.6"
		TARGET_EXTRA="$TARGET_DEVICE/extra"
		TARGET_ROOTFS="$TARGET_DEVICE/rootfs"
	else
		echo "=== not find or error of the .buildconfig ==="
		\cd $T/ && ./build.sh config
	fi
}

build_usage
main
