function build_yocto_rootfs()
{
	local yocto_rootdir=${LICHEE_TOP_DIR}/yocto

	if [ -z "${LICHEE_YOCTO_MACHINE}" -o \
	     -z "${LICHEE_YOCTO_DISTRO}" -o \
	     -z "${LICHEE_YOCTO_IMAGE}" ];then
		mk_error "yocto machine:${LICHEE_YOCTO_MACHINE},distro:${LICHEE_YOCTO_DISTRO},image:${LICHEE_YOCTO_IMAGE}"
		return 1
	fi

	echo "==mkcmd.sh: build_yocto_rootfs $@=="

	cd  ${yocto_rootdir}
	echo "MACHINE=${LICHEE_YOCTO_MACHINE} DISTRO=${LICHEE_YOCTO_DISTRO}  source setup-environment build"
	MACHINE=${LICHEE_YOCTO_MACHINE} DISTRO=${LICHEE_YOCTO_DISTRO}  source setup-environment build
	bitbake ${LICHEE_YOCTO_IMAGE}
	[ $? -ne 0 ] && mk_error "yocto build Failed" && return 1

	[ -e $LICHEE_PLAT_OUT/build ] && rm -f $LICHEE_PLAT_OUT/build
	ln -sr ${yocto_rootdir}/build $LICHEE_PLAT_OUT/build

	rm -f $LICHEE_PLAT_OUT/rootfs.*
	ln -s $LICHEE_PLAT_OUT/build/tmp/deploy/images/${LICHEE_YOCTO_MACHINE}/${LICHEE_YOCTO_IMAGE}-${LICHEE_YOCTO_MACHINE}.rootfs.ext4 $LICHEE_PLAT_OUT/rootfs.ext4

	ln -s $LICHEE_PLAT_OUT/build/tmp/deploy/images/${LICHEE_YOCTO_MACHINE}/${LICHEE_YOCTO_IMAGE}-${LICHEE_YOCTO_MACHINE}.rootfs.squashfs-xz $LICHEE_PLAT_OUT/rootfs.squashfs

	ln -s $LICHEE_PLAT_OUT/build/tmp/deploy/images/${LICHEE_YOCTO_MACHINE}/${LICHEE_YOCTO_IMAGE}-${LICHEE_YOCTO_MACHINE}.rootfs.ext4 $LICHEE_PLAT_OUT/rootfs.img

	return $?
}

function clyocto()
{
	local yocto_rootdir=${LICHEE_TOP_DIR}/yocto

	[ -d ${yocto_rootdir}/build ] && rm -rf ${yocto_rootdir}/build

	return $?
}



