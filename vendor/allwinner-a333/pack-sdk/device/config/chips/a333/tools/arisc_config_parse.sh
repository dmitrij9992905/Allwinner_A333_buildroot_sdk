#!/bin/bash

#set -e

ARISC_CONFIG_FILE=${LICHEE_BOARD_CONFIG_DIR}/arisc.config

pmu_type_parse()
{
	PMU_TYPE=`awk '/eta,eta/{print $3;exit;}' ${LICHEE_BOARD_CONFIG_DIR}/board.dts`

	rm -f $ARISC_CONFIG_FILE
	touch $ARISC_CONFIG_FILE
	echo "export LICHEE_ARISC_DEFDIR=ar100s" >> $ARISC_CONFIG_FILE
	echo "export LICHEE_ARISC_DEFCONFIG=sun65iw1p1_defconfig" >> $ARISC_CONFIG_FILE
}

pmu_type_parse

