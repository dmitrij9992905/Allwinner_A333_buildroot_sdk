#!/bin/bash

function prepare_tools()
{
	local toolspath=$(load_config LICHEE_OUT_DIR "$configlist")/pack-hook-bin

	export PATH=$toolspath/bin:$PATH
	export LD_LIBRARY=$toolspath/lib64:$toolspath/lib:$LD_LIBRARY

	if [ -d $toolspath ]; then
		if [ -n "$(which phtb-version)" ]; then
			local version="$(phtb-version read)"
			local current="$(phtb-version calc)"
			[ "$current" == "$version" ] && return 0
		fi
		rm -rf $toolspath
	fi

	if [ ! -d $toolspath ]; then
		mkdir -p $toolspath
		tar -vzxf $(cd $(dirname ${BASH_SOURCE[0]}) && pwd)/tools/prebuilt.tar.gz -C $toolspath
	fi
}

function get_raw_data()
{
	local src=$1
	local dst=$2
	local offset=$3
	local size=$4
	local bs=4096

	local skip_num skip_rem size_num size_rem size_cur

	if [ $size -le $bs ]; then
		dd if=$src of=$dst bs=1 skip=$offset count=$size status=none
		return 0
	fi

	skip_num=$((offset/bs))
	skip_rem=$((offset%bs))

	if [ $skip_rem -ne 0 ]; then
		size_num=$(((size-bs+skip_rem)/bs))
		size_rem=$(((size-bs+skip_rem)%bs))
		dd if=$src of=$dst bs=1 skip=$offset count=$((bs-skip_rem)) status=none
		dd if=$src of=$dst bs=$bs skip=$((skip_num+1)) count=$size_num oflag=append conv=notrunc status=none
		if [ $size_rem -ne 0 ]; then
			dd if=$src of=$dst bs=1 skip=$(((skip_num+1+size_num)*bs)) count=$size_rem oflag=append conv=notrunc status=none
		fi
	else
		size_num=$((size/bs))
		size_rem=$((size%bs))
		dd if=$src of=$dst bs=$bs skip=$skip_num count=$size_num status=none
		if [ $size_rem -ne 0 ]; then
			dd if=$src of=$dst bs=1 skip=$(((skip_num+size_num)*bs)) count=$size_rem oflag=append conv=notrunc status=none
		fi
	fi
}

function extract_data()
{
	local fw=$1
	local out=$2
	local magic="0001000000040000"
	local offset main sub name size ofs
	local outname

	for ((i=0;i<256;i++)); do
		offset=$((i*1024))
		if [ "x$(hexdump -s $offset -n 8 -v -e '8/1 "%02X"' $fw)" == "x$magic" ]; then
			main=$(hexdump -s $((offset+8)) -n 8 -v -e '"%c"' $fw)
			sub=$(hexdump -s $((offset+16)) -n 16 -v -e '"%c"' $fw | sed 's|\x00\+||g')
			name=$(hexdump -s $((offset+36)) -n 32 -v -e '"%c"' $fw | sed 's|\x00\+||g')
			size=$(hexdump -s $((offset+300)) -n 4 -v -e '4/1 "%02X"' $fw)
			size=$(printf "%d" 0x${size:6:2}${size:4:2}${size:2:2}${size:0:2})
			ofs=$(hexdump -s $((offset+308)) -n 4 -v -e '4/1 "%02X"' $fw)
			ofs=$(printf "%d" 0x${ofs:6:2}${ofs:4:2}${ofs:2:2}${ofs:0:2})

			LOGD "$(printf "[%-8s, %-16s]: %-18s, offset: %08X, size: %08X\n" "$main" "$sub" "$name" $ofs $size)"
			unset outname
			case "$main" in
				RFSFAT16)
					[ $size -gt 4 ] && outname=${name/%.fex/.img}
					;;
				*)
					[ "$name" == "sys_partition.fex" -o "$name" == "sys_config.fex" ] && outname=${name}
					[ "$name" == "dragon_toc.fex" ] && outname=dragon_toc.cfg
					;;
			esac
			[ -n "$outname" ] && get_raw_data "$fw" "$out/$outname" $ofs $size
		fi
	done
}

function unpack_img()
{
	local fw=$1
	local out=$2
	local tmp=$out/tmp

	mkdir -p $out $tmp
	find $out $tmp -type f | xargs rm -rf

	extract_data $fw $out
	unpack_super $out/super.img $out
	unpack_env   $out/env.img   $out
}

function get_fs_type()
{
	local img=$1

	if [ "$(hexdump -s 1024 -n 4 -v -e '4/1 "%02X"' $img)" == "E2E1F5E0" ]; then
		echo "erofs"
	elif [ "$(hexdump -n 8 -v -e '8/1 "%02X"' $img)" == "3AFF26ED01000000" ]; then
		echo "sparse"
	fi
}

function unpack_super()
{
	local img=$1
	local out=$2
	local tmp=$out/tmp

	([ -f $img ] && simg2img $img $tmp/super.ext4 && lpunpack $tmp/super.ext4 $out)
	local list=$(load_config BOARD_SB_PARTITION_LIST "$configlist")
	for f in $list; do
		[ -f $out/${f}_a.img ] && mv $out/${f}_a.img $out/${f}.img
		rm -rf $out/${f}_b.img
	done
}

function unpack_boot()
{
	local img=$1
	local out=$2
	rm -rf $out
	mkdir -p $out
	echo "boot image: $img"  > $out/unpack_boot.info
	echo "out: $out"        >> $out/unpack_boot.info
	wrapper_run_logd unpack_bootimg.py --boot_img $img --out $out 2>&1 | tee -a $out/unpack_boot.info
}

function unpack_env()
{
	local img=$1
	local out=$2
	local size=$(get_img_size $img)

	get_raw_data $img $out/env.cfg 4 $((size-4))
	sed -i 's|\x00|\x0a|g' $out/env.cfg
	sed -i '/^$/d' $out/env.cfg
}

function load_config()
{
	local cfgkey=$1
	local cfgfile=""
	local defval=$3
	local val=""

	for f in $2; do
		[ -f $f ] && cfgfile+=" $f"
	done

	[ -n "$cfgfile" ] && val="$(awk -F= '/^(export)*[[:space:]]*'"$cfgkey"'=/{print $2}' $cfgfile | tail -n 1)"
	eval echo "${val:-"$defval"}"
}

function get_img_size()
{
	local image_i=$(readlink -f $1)
	local image_t=$(dirname $image_i)/.get-img-size.ext4
	local magic=$(hexdump -n 8 -v -e '8/1 "%02X"' $image_i)

	case "$magic" in
		"3AFF26ED01000000")
			simg2img $image_i $image_t
			stat $image_t --format="%s"
			;;
		*)
			stat $image_i --format="%s"
			;;
	esac

	rm -rf $image_t
}

function make_dtbo()
{
	local BOARD_PREBUILT_DTBOIMAGE=$(load_config LICHEE_PLAT_OUT $configlist)/dtbo.img
	[ ! -f $prebuiltpath/unpack/dtbo.img ] && return 0
	[ ! -f $BOARD_PREBUILT_DTBOIMAGE ] && return 0

	local AVBTOOL=avbtool
	local BUILD_FINGERPRINT=$(strings $prebuiltpath/unpack/dtbo.img | grep com.android.build.dtbo.fingerprint -A 2 | grep 'Allwinner/.*-keys')
	local INSTALLED_DTBOIMAGE_TARGET=$prebuiltpath/dtbo.img
	local BOARD_DTBOIMG_PARTITION_SIZE=$(get_img_size $prebuiltpath/unpack/dtbo.img)
	local INTERNAL_AVB_DTBO_SIGNING_ARGS=""
	local BOARD_AVB_DTBO_ADD_HASH_FOOTER_ARGS="--prop com.android.build.dtbo.fingerprint:${BUILD_FINGERPRINT}"

	cp ${BOARD_PREBUILT_DTBOIMAGE} ${INSTALLED_DTBOIMAGE_TARGET}

	${AVBTOOL} add_hash_footer \
		--image ${INSTALLED_DTBOIMAGE_TARGET} \
		--partition_size ${BOARD_DTBOIMG_PARTITION_SIZE} \
		--partition_name dtbo ${INTERNAL_AVB_DTBO_SIGNING_ARGS} \
		${BOARD_AVB_DTBO_ADD_HASH_FOOTER_ARGS}
}

function make_vbmeta_system()
{
	local prebuilt_path=$prebuiltpath

	local AVBTOOL=avbtool

	local BOARD_AVB_VBMETA_SYSTEM_ALGORITHM=$(load_config BOARD_AVB_VBMETA_SYSTEM_ALGORITHM "$configlist")
	local BOARD_AVB_VBMETA_SYSTEM_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_VBMETA_SYSTEM_KEY_PATH "$configlist")

	local PLATFORM_SECURITY_PATCH=$(load_config ro.build.version.security_patch "$configlist")
	local BOARD_AVB_VBMETA_SYSTEM_ROLLBACK_INDEX=$(date -d 'TZ="GMT" '"${PLATFORM_SECURITY_PATCH}" +%s)

	local INTERNAL_AVB_VBMETA_SYSTEM_SIGNING_ARGS="--algorithm ${BOARD_AVB_VBMETA_SYSTEM_ALGORITHM} --key ${BOARD_AVB_VBMETA_SYSTEM_KEY_PATH}"
	local INTERNAL_AVB_MAKE_VBMETA_SYSTEM_IMAGE_ARGS="--include_descriptors_from_image $prebuilt_path/system.img"
	local BOARD_AVB_MAKE_VBMETA_SYSTEM_IMAGE_ARGS="--padding_size 4096"
	BOARD_AVB_MAKE_VBMETA_SYSTEM_IMAGE_ARGS+=" --rollback_index ${BOARD_AVB_VBMETA_SYSTEM_ROLLBACK_INDEX}"

	rm -rf $prebuilt_path/vbmeta_system.img
	${AVBTOOL} make_vbmeta_image \
		${INTERNAL_AVB_VBMETA_SYSTEM_SIGNING_ARGS} \
		${BOARD_AVB_MAKE_VBMETA_SYSTEM_IMAGE_ARGS} \
		${INTERNAL_AVB_MAKE_VBMETA_SYSTEM_IMAGE_ARGS} \
		--output $prebuilt_path/vbmeta_system.img
}

function make_vbmeta_vendor()
{
	local prebuilt_path=$prebuiltpath

	local AVBTOOL=avbtool

	local BOARD_AVB_VBMETA_VENDOR_ALGORITHM=$(load_config BOARD_AVB_VBMETA_SYSTEM_ALGORITHM "$configlist")
	local BOARD_AVB_VBMETA_VENDOR_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_VBMETA_SYSTEM_KEY_PATH "$configlist")

	local PLATFORM_SECURITY_PATCH=$(load_config ro.build.version.security_patch "$configlist")
	local BOARD_AVB_VBMETA_VENDOR_ROLLBACK_INDEX=$(date -d 'TZ="GMT" '"${PLATFORM_SECURITY_PATCH}" +%s)

	local INTERNAL_AVB_VBMETA_VENDOR_SIGNING_ARGS="--algorithm ${BOARD_AVB_VBMETA_VENDOR_ALGORITHM} --key ${BOARD_AVB_VBMETA_VENDOR_KEY_PATH}"
	local INTERNAL_AVB_MAKE_VBMETA_VENDOR_IMAGE_ARGS="--include_descriptors_from_image $prebuilt_path/vendor.img"
	local BOARD_AVB_MAKE_VBMETA_VENDOR_IMAGE_ARGS="--padding_size 4096"
	BOARD_AVB_MAKE_VBMETA_VENDOR_IMAGE_ARGS+=" --rollback_index ${BOARD_AVB_VBMETA_VENDOR_ROLLBACK_INDEX}"

	rm -rf $prebuilt_path/vbmeta_vendor.img
	${AVBTOOL} make_vbmeta_image \
		${INTERNAL_AVB_VBMETA_VENDOR_SIGNING_ARGS} \
		${BOARD_AVB_MAKE_VBMETA_VENDOR_IMAGE_ARGS} \
		${INTERNAL_AVB_MAKE_VBMETA_VENDOR_IMAGE_ARGS} \
		--output $prebuilt_path/vbmeta_vendor.img
}

function make_vbmeta()
{
	local prebuilt_path=$prebuiltpath
	local local_avb_key=$prebuilt_path/avb_chain_keys

	local AVBTOOL=avbtool

	local BOARD_AVB_VBMETA_SYSTEM_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_VBMETA_SYSTEM_KEY_PATH "$configlist")
	local BOARD_AVB_VBMETA_VENDOR_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_VBMETA_VENDOR_KEY_PATH "$configlist")
	local BOARD_AVB_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_KEY_PATH "$configlist")

	local BOARD_AVB_ALGORITHM=$(load_config BOARD_AVB_ALGORITHM "$configlist")
	local list=($(load_config BOARD_AVB_INCLUDE_PARTITON "$configlist"))

	list+=($(strings $prebuilt_path/unpack/vbmeta.img | grep -o 'com.android.build.\S\+.fingerprint' | sed 's|com.android.build.\(\S\+\).fingerprint|\1|g'))
	list=($(xargs -n 1 <<< ${list[@]} | sort | uniq))

	# extract-avb-chain-public-keys
	mkdir -p $local_avb_key
	if [ -n "$BOARD_AVB_VBMETA_SYSTEM_KEY_PATH" ]; then
		${AVBTOOL} extract_public_key --key ${BOARD_AVB_VBMETA_SYSTEM_KEY_PATH} \
			--output $local_avb_key/vbmeta_system.avbpubkey
	fi

	if [ -n "$BOARD_AVB_VBMETA_VENDOR_KEY_PATH" ]; then
		${AVBTOOL} extract_public_key --key ${BOARD_AVB_VBMETA_VENDOR_KEY_PATH} \
			--output $local_avb_key/vbmeta_vendor.avbpubkey
	fi

	# build-vbmetaimage-target
	local PRIVATE_AVB_VBMETA_SIGNING_ARGS="--algorithm ${BOARD_AVB_ALGORITHM} --key ${BOARD_AVB_KEY_PATH}"
	local BOARD_AVB_MAKE_VBMETA_IMAGE_ARGS="--padding_size 4096"
	local INTERNAL_AVB_MAKE_VBMETA_IMAGE_ARGS=""

	for p in ${list[@]}; do
		if [ ! -f $prebuilt_path/${p}.img ]; then
			LOGW "${FUNCNAME[0]}: ${p}.img not found, ignore"
			continue
		fi

		${AVBTOOL} info_image --image $prebuilt_path/${p}.img > $prebuilt_path/info 2>&1 || true
		if [ -n "$(grep "Given image does not look like a vbmeta image" $prebuilt_path/info)" ]; then
			LOGW "${FUNCNAME[0]}: ${p}.img maybe not a vbmeta image, ignore"
			continue
		fi

		INTERNAL_AVB_MAKE_VBMETA_IMAGE_ARGS+=" --include_descriptors_from_image $prebuilt_path/${p}.img"
	done
	rm -rf $prebuilt_path/info

	INTERNAL_AVB_MAKE_VBMETA_IMAGE_ARGS+=" --chain_partition vbmeta_system:1:$local_avb_key/vbmeta_system.avbpubkey"
	INTERNAL_AVB_MAKE_VBMETA_IMAGE_ARGS+=" --chain_partition vbmeta_vendor:2:$local_avb_key/vbmeta_vendor.avbpubkey"

	rm -rf $prebuilt_path/vbmeta.img

	${AVBTOOL} make_vbmeta_image \
		${INTERNAL_AVB_MAKE_VBMETA_IMAGE_ARGS} \
		${PRIVATE_AVB_VBMETA_SIGNING_ARGS} \
		${BOARD_AVB_MAKE_VBMETA_IMAGE_ARGS} \
		--output $prebuilt_path/vbmeta.img

	rm -rf $local_avb_key
}

function make_super()
{
	local prebuilt_path=$prebuiltpath
	local override_path=$overridepath

	local BOARD_SUPER_RESERVED_SIZE=$(load_config BOARD_SUPER_RESERVED_SIZE "$configlist")
	local BOARD_SB_PARTITION_LIST=$(load_config BOARD_SB_PARTITION_LIST "$configlist")
	local ab_update=$(load_config PRODUCT_VIRTUAL_AB_OTA "$configlist")

	local BOARD_SUPER_PARTITION_SIZE=$(get_img_size $prebuilt_path/unpack/super.img)
	local BOARD_SB_SIZE=$((BOARD_SUPER_PARTITION_SIZE-BOARD_SUPER_RESERVED_SIZE))

	local partion_size=""
	local current_size=""

	local lpmake_args="--metadata-size 65536 --super-name super"

	if [ "$ab_update" == "true" ]; then
		lpmake_args+=" --metadata-slots 3 --virtual-ab"
		lpmake_args+=" --device super:${BOARD_SUPER_PARTITION_SIZE}"
		lpmake_args+=" --group sb_a:${BOARD_SB_SIZE}"
		lpmake_args+=" --group sb_b:${BOARD_SB_SIZE}"
		for p in $BOARD_SB_PARTITION_LIST; do
			[ -f $override_path/${p}.img ] && \
			partion_size=$(get_img_size $override_path/${p}.img) || \
			partion_size=$(get_img_size $prebuilt_path/unpack/${p}.img)
			current_size=$(get_img_size $prebuilt_path/${p}.img)
			[ $current_size -gt $partion_size ] && partion_size=$current_size
			lpmake_args+=" --partition ${p}_a:readonly:${partion_size}:sb_a --image ${p}_a=$prebuilt_path/${p}.img"
			lpmake_args+=" --partition ${p}_b:readonly:0:sb_b"
		done
	else
		lpmake_args+=" --metadata-slots 2"
		lpmake_args+=" --device super:${BOARD_SUPER_PARTITION_SIZE}"
		lpmake_args+=" --group sb:${BOARD_SB_SIZE}"
		for p in $BOARD_SB_PARTITION_LIST; do
			[ -f $override_path/${p}.img ] && \
			partion_size=$(get_img_size $override_path/${p}.img) || \
			partion_size=$(get_img_size $prebuilt_path/unpack/${p}.img)
			current_size=$(get_img_size $prebuilt_path/${p}.img)
			[ $current_size -gt $partion_size ] && partion_size=$current_size
			lpmake_args+=" --partition ${p}:readonly:${partion_size}:sb --image ${p}=$prebuilt_path/${p}.img"
		done
	fi

	lpmake_args+=" --sparse --output $prebuilt_path/super.img"
	LOGD "${FUNCNAME[0]}: lpmake ${lpmake_args}"

	rm -rf $prebuilt_path/super*.img
	wrapper_run_logd lpmake ${lpmake_args}
}

function make_ramdisk()
{
	local action=$1
	local src=$2
	local dst=$3
	local filetype=$4

	local compresstools=gzip
	local compressargs=""

	if [ "$filetype" != "gzip" ]; then
		compresstools=lz4
		compressargs="-l -12 --favor-decSpeed"
	fi

	case $action in
		e)
			# src is file & dst is path
			rm -rf $dst
			mkdir -p $dst
			$compresstools -dc $src | (cd $dst; wrapper_run_logd fakeroot cpio -i)
			;;
		c)
			# src is path & dst is file
			rm -rf $dst
			[ ! -d $src ] && LOGE "src not exist" && return 1
			(cd $src && find . | fakeroot cpio -o -Hnewc | $compresstools $compressargs > $dst)
			;;
	esac
}

function modify_system()
{
	LOGD "${FUNCNAME[0]}: not implement currently"
	return 0

	# Must re-add hashtree footer after modify
	local AVBTOOL=avbtool
	local OS_FINGERPRINT=$(load_config ro.system.build.fingerprint "$configlist")
	local OS_PATCH_LEVEL=$(load_config ro.build.version.security_patch "$configlist")
	local OS_VERSION=$(load_config ro.build.version.release "$configlist")
	local BOARD_AVB_SYSTEM_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_SYSTEM_KEY_PATH "$configlist")
	local BOARD_AVB_SYSTEM_ALGORITHM=$(load_config BOARD_AVB_SYSTEM_ALGORITHM "$configlist")
	local BOARD_AVB_SYSTEM_PARTITION_SIZE=0 # 0 for auto calc
	local INTERNAL_AVB_CUSTOMIMAGES_SIGNING_ARGS=""
	local BOARD_AVB_SYSTEM_ADD_HASHTREE_FOOTER_ARGS="--prop com.android.build.system.fingerprint:${OS_FINGERPRINT}"
	BOARD_AVB_SYSTEM_ADD_HASHTREE_FOOTER_ARGS+=" --prop com.android.build.system.os_version:${OS_VERSION}"
	BOARD_AVB_SYSTEM_ADD_HASHTREE_FOOTER_ARGS+=" --prop com.android.build.system.security_patch:${OS_PATCH_LEVEL}"

	local AVB_HASHTREE_FOOTER_ARGS=$(load_config BOARD_AVB_SYSTEM_ADD_HASHTREE_FOOTER_ARGS "$configlist")
	BOARD_AVB_SYSTEM_ADD_HASHTREE_FOOTER_ARGS+=" $AVB_HASHTREE_FOOTER_ARGS"

	local AVB_SIGNING_ARGS=""
	[ -n "$BOARD_AVB_SYSTEM_KEY_PATH" ] && \
	AVB_SIGNING_ARGS+=" --key ${BOARD_AVB_SYSTEM_KEY_PATH}"

	[ -n "$BOARD_AVB_SYSTEM_ALGORITHM" ] && \
	AVB_SIGNING_ARGS+=" --algorithm ${BOARD_AVB_SYSTEM_ALGORITHM}"

	wrapper_run_logd ${AVBTOOL} add_hashtree_footer \
		--image $prebuiltpath/system.img \
		${AVB_SIGNING_ARGS} \
		--partition_size ${BOARD_AVB_SYSTEM_PARTITION_SIZE} \
		--partition_name system \
		${INTERNAL_AVB_CUSTOMIMAGES_SIGNING_ARGS} \
		${BOARD_AVB_SYSTEM_ADD_HASHTREE_FOOTER_ARGS}
}

function modify_vendor()
{
	local BOARD_SB_PARTITION_LIST=$(load_config BOARD_SB_PARTITION_LIST "$configlist")
	if [[ "$BOARD_SB_PARTITION_LIST" =~ vendor_dlkm ]]; then
		LOGD "${FUNCNAME[0]}: image has vendor_dlkm partition, ignore vendor modify currently"
		return 0
	fi

	local kmodpath=$(load_config LICHEE_PLAT_OUT "$configlist")/lib/modules/*/
	local imodpath=$(load_config BOARD_MODULE_PATH "$configlist")
	local image=$prebuiltpath/vendor.img
	local newsize=$(($(du --apparent-size -sm $image | awk '{print $1}')+8))
	e2fsck -y $image
	resize2fs $image ${newsize}M
	local list=$(e2ls ${image}:/$imodpath)
	for f in $list; do
		if [ -f $kmodpath/$f ]; then
			e2cp -P 644 -O 0 -G 0 $kmodpath/$f ${image}:/$imodpath/
		fi
	done

	# Must re-add hashtree footer after modify
	local AVBTOOL=avbtool
	local OS_FINGERPRINT=$(load_config ro.system.build.fingerprint "$configlist")
	local OS_VERSION=$(load_config ro.build.version.release "$configlist")
	local BOARD_AVB_VENDOR_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_VENDOR_KEY_PATH "$configlist")
	local BOARD_AVB_VENDOR_ALGORITHM=$(load_config BOARD_AVB_VENDOR_ALGORITHM "$configlist")
	local BOARD_AVB_VENDOR_PARTITION_SIZE=0 # 0 for auto calc
	local INTERNAL_AVB_CUSTOMIMAGES_SIGNING_ARGS=""
	local BOARD_AVB_VENDOR_ADD_HASHTREE_FOOTER_ARGS="--prop com.android.build.vendor.fingerprint:${OS_FINGERPRINT}"
	BOARD_AVB_VENDOR_ADD_HASHTREE_FOOTER_ARGS+=" --prop com.android.build.vendor.os_version:${OS_VERSION}"

	local AVB_HASHTREE_FOOTER_ARGS=$(load_config BOARD_AVB_VENDOR_ADD_HASHTREE_FOOTER_ARGS "$configlist")
	BOARD_AVB_VENDOR_ADD_HASHTREE_FOOTER_ARGS+=" $AVB_HASHTREE_FOOTER_ARGS"

	local AVB_SIGNING_ARGS=""
	[ -n "$BOARD_AVB_VENDOR_KEY_PATH" ] && \
	AVB_SIGNING_ARGS+=" --key ${BOARD_AVB_VENDOR_KEY_PATH}"

	[ -n "$BOARD_AVB_VENDOR_ALGORITHM" ] && \
	AVB_SIGNING_ARGS+=" --algorithm ${BOARD_AVB_VENDOR_ALGORITHM}"

	wrapper_run_logd ${AVBTOOL} add_hashtree_footer \
		--image $prebuiltpath/vendor.img \
		${AVB_SIGNING_ARGS} \
		--partition_size ${BOARD_AVB_VENDOR_PARTITION_SIZE} \
		--partition_name vendor \
		${INTERNAL_AVB_CUSTOMIMAGES_SIGNING_ARGS} \
		${BOARD_AVB_VENDOR_ADD_HASHTREE_FOOTER_ARGS}
}

function modify_system_dlkm()
{
	[ -f $overridepath/system_dlkm.img ] && return 0

	local kmodpath=$(load_config LICHEE_PLAT_OUT "$configlist")/lib/modules/*/
	local imodpath=$(load_config BOARD_MODULE_PATH "$configlist")
	local image=$prebuiltpath/system_dlkm.img
	local fstype=$(get_fs_type $image)

	if [ "$fstype" == "erofs" ]; then
		local extractpath=$prebuiltpath/system_dlkm-extract
		rm -rf $extractpath
		wrapper_run_logd fsck.erofs --extract=$extractpath --force $image
		local list=$(ls $extractpath/$imodpath)
		for f in $list; do
			if [ -f $kmodpath/$f ]; then
				cp $kmodpath/$f $extractpath/$imodpath/
				chmod 644 $extractpath/$imodpath/$f
			fi
		done
		rm -rf $image
		local erofs_default_compressor="$(load_config BOARD_SYSTEM_DLKM_EROFS_DEFAULT_COMPRESSOR "$configlist")"
		local selinux_fc=$lichee_top_path/$(load_config BOARD_SELINUX_FC "$configlist")
		wrapper_run_logd mkfs.erofs -z $erofs_default_compressor \
				--mount-point=system_dlkm \
				--file-contexts=$selinux_fc \
				--force-uid=0 \
				--force-gid=0 \
				$image $extractpath
		rm -rf $extractpath
	else
		local newsize=$(($(du --apparent-size -sm $image | awk '{print $1}')+8))
		local orign=${image}.orig

		mv ${image} ${orign}

		wrapper_run_logd dd if=/dev/zero of=${image} bs=1M count=$newsize
		wrapper_run_logd mkfs.ext4 -F ${image} -b 4096
		wrapper_run_logd e2fsck -y ${image}

		local list=$(e2ls ${orign}:/$imodpath)
		rm -rf $prebuiltpath/tmp
		mkdir -p $prebuiltpath/tmp/$imodpath
		for f in $list; do
			e2cp ${orign}:/$imodpath/$f $prebuiltpath/tmp/$imodpath/$f
			if [ -f $kmodpath/$f ]; then
				cp $kmodpath/$f $prebuiltpath/tmp/$imodpath/$f
			else
				e2cp ${orign}:/$imodpath/$f $prebuiltpath/tmp/$imodpath/$f
			fi
		done
		local selinux_fc=$lichee_top_path/$(load_config BOARD_SELINUX_FC "$configlist")
		wrapper_run_logd e2fsdroid -e -f $prebuiltpath/tmp -a /system_dlkm -S $selinux_fc ${image}
		rm -rf $prebuiltpath/tmp
	fi

	# Must re-add hashtree footer after modify
	local AVBTOOL=avbtool
	local OS_FINGERPRINT=$(load_config ro.system.build.fingerprint "$configlist")
	local OS_VERSION=$(load_config ro.build.version.release "$configlist")
	local BOARD_AVB_SYSTEM_DLKM_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_SYSTEM_DLKM_KEY_PATH "$configlist")
	local BOARD_AVB_SYSTEM_DLKM_ALGORITHM=$(load_config BOARD_AVB_SYSTEM_DLKM_ALGORITHM "$configlist")
	local BOARD_AVB_SYSTEM_DLKM_PARTITION_SIZE=0 # 0 for auto calc
	local INTERNAL_AVB_CUSTOMIMAGES_SIGNING_ARGS=""
	local BOARD_AVB_SYSTEM_DLKM_ADD_HASHTREE_FOOTER_ARGS="--prop com.android.build.system_dlkm.fingerprint:${OS_FINGERPRINT}"
	BOARD_AVB_SYSTEM_DLKM_ADD_HASHTREE_FOOTER_ARGS+=" --prop com.android.build.system_dlkm.os_version:${OS_VERSION}"

	local AVB_HASHTREE_FOOTER_ARGS=$(load_config BOARD_AVB_SYSTEM_DLKM_ADD_HASHTREE_FOOTER_ARGS "$configlist")
	BOARD_AVB_SYSTEM_DLKM_ADD_HASHTREE_FOOTER_ARGS+=" $AVB_HASHTREE_FOOTER_ARGS"

	local AVB_SIGNING_ARGS=""
	[ -n "$BOARD_AVB_SYSTEM_DLKM_KEY_PATH" ] && \
	AVB_SIGNING_ARGS+=" --key ${BOARD_AVB_SYSTEM_DLKM_KEY_PATH}"

	[ -n "$BOARD_AVB_SYSTEM_DLKM_ALGORITHM" ] && \
	AVB_SIGNING_ARGS+=" --algorithm ${BOARD_AVB_SYSTEM_DLKM_ALGORITHM}"

	wrapper_run_logd ${AVBTOOL} add_hashtree_footer \
		--image $prebuiltpath/system_dlkm.img \
		${AVB_SIGNING_ARGS} \
		--partition_size ${BOARD_AVB_SYSTEM_DLKM_PARTITION_SIZE} \
		--partition_name system_dlkm \
		${INTERNAL_AVB_CUSTOMIMAGES_SIGNING_ARGS} \
		${BOARD_AVB_SYSTEM_DLKM_ADD_HASHTREE_FOOTER_ARGS}
}

function modify_vendor_dlkm()
{
	local kmodpath=$(load_config LICHEE_PLAT_OUT "$configlist")/lib/modules/*/
	local imodpath=$(load_config BOARD_MODULE_PATH "$configlist")
	local image=$prebuiltpath/vendor_dlkm.img
	local fstype=$(get_fs_type $image)

	if [ "$fstype" == "erofs" ]; then
		local extractpath=$prebuiltpath/vendor_dlkm-extract
		rm -rf $extractpath
		wrapper_run_logd fsck.erofs --extract=$extractpath --force $image
		local list=$(ls $extractpath/$imodpath)
		for f in $list; do
			if [ -f $kmodpath/$f ]; then
				cp $kmodpath/$f $extractpath/$imodpath/
				chmod 644 $extractpath/$imodpath/$f
			fi
		done
		rm -rf $image
		local erofs_default_compressor="$(load_config BOARD_VENDOR_DLKM_EROFS_DEFAULT_COMPRESSOR "$configlist")"
		local selinux_fc=$lichee_top_path/$(load_config BOARD_SELINUX_FC "$configlist")
		mkfs.erofs -z $erofs_default_compressor \
				--mount-point=vendor_dlkm \
				--file-contexts=$selinux_fc \
				--force-uid=0 \
				--force-gid=0 \
				$image $extractpath \
				2>&1 | while read  msg; do LOGD "$msg"; done
		rm -rf $extractpath
	else
		local newsize=$(($(du --apparent-size -sm $image | awk '{print $1}')+8))
		wrapper_run_logd e2fsck -y $image
		wrapper_run_logd resize2fs $image ${newsize}M
		local list=$(e2ls ${image}:/$imodpath)
		for f in $list; do
			if [ -f $kmodpath/$f ]; then
				e2cp -P 644 -O 0 -G 0 $kmodpath/$f ${image}:/$imodpath/
			fi
		done
	fi

	# Must re-add hashtree footer after modify
	local AVBTOOL=avbtool
	local OS_FINGERPRINT=$(load_config ro.system.build.fingerprint "$configlist")
	local OS_VERSION=$(load_config ro.build.version.release "$configlist")
	local BOARD_AVB_VENDOR_DLKM_KEY_PATH=$lichee_top_path/$(load_config BOARD_AVB_VENDOR_DLKM_KEY_PATH "$configlist")
	local BOARD_AVB_VENDOR_DLKM_ALGORITHM=$(load_config BOARD_AVB_VENDOR_DLKM_ALGORITHM "$configlist")
	local BOARD_AVB_VENDOR_DLKM_PARTITION_SIZE=0 # 0 for auto calc
	local INTERNAL_AVB_CUSTOMIMAGES_SIGNING_ARGS=""
	local BOARD_AVB_VENDOR_DLKM_ADD_HASHTREE_FOOTER_ARGS="--prop com.android.build.vendor_dlkm.fingerprint:${OS_FINGERPRINT}"
	BOARD_AVB_VENDOR_DLKM_ADD_HASHTREE_FOOTER_ARGS+=" --prop com.android.build.vendor_dlkm.os_version:${OS_VERSION}"

	local AVB_HASHTREE_FOOTER_ARGS=$(load_config BOARD_AVB_VENDOR_DLKM_ADD_HASHTREE_FOOTER_ARGS "$configlist")
	BOARD_AVB_VENDOR_DLKM_ADD_HASHTREE_FOOTER_ARGS+=" $AVB_HASHTREE_FOOTER_ARGS"

	local AVB_SIGNING_ARGS=""
	[ -n "$BOARD_AVB_VENDOR_DLKM_KEY_PATH" ] && \
	AVB_SIGNING_ARGS+=" --key ${BOARD_AVB_VENDOR_DLKM_KEY_PATH}"

	[ -n "$BOARD_AVB_VENDOR_DLKM_ALGORITHM" ] && \
	AVB_SIGNING_ARGS+=" --algorithm ${BOARD_AVB_VENDOR_DLKM_ALGORITHM}"

	wrapper_run_logd ${AVBTOOL} add_hashtree_footer \
		--image $prebuiltpath/vendor_dlkm.img \
		${AVB_SIGNING_ARGS} \
		--partition_size ${BOARD_AVB_VENDOR_DLKM_PARTITION_SIZE} \
		--partition_name vendor_dlkm \
		${INTERNAL_AVB_CUSTOMIMAGES_SIGNING_ARGS} \
		${BOARD_AVB_VENDOR_DLKM_ADD_HASHTREE_FOOTER_ARGS}
}

function modify_product()
{
	LOGD "${FUNCNAME[0]}: not implement currently"
	return 0
}
