#!/bin/bash

loglevel=${LOG_LEVEL:-4}

function LOGF()
{
	printf "$(date "+%m-%d %H:%M:%S.%3N") %7d F %-10s: \033[47;31m%b\033[0m\n" "$$" "${LOG_TAG:0:10}" "$*"
	exit 1
}

function LOGE()
{
	[ $loglevel -ge 1 ] && \
	printf "$(date "+%m-%d %H:%M:%S.%3N") %7d E %-10s: \033[47;31m%b\033[0m\n" "$$" "${LOG_TAG:0:10}" "$*"
}

function LOGW()
{
	[ $loglevel -ge 2 ] && \
	printf "$(date "+%m-%d %H:%M:%S.%3N") %7d W %-10s: \033[47;34m%b\033[0m\n" "$$" "${LOG_TAG:0:10}" "$*"
}

function LOGI()
{
	[ $loglevel -ge 3 ] && \
	printf "$(date "+%m-%d %H:%M:%S.%3N") %7d I %-10s: \033[47;30m%b\033[0m\n" "$$" "${LOG_TAG:0:10}" "$*"
}

function LOGD()
{
	[ $loglevel -ge 4 ] && \
	printf "$(date "+%m-%d %H:%M:%S.%3N") %7d D %-10s: %b\n" "$$" "${LOG_TAG:0:10}" "$*"
}

function LOGV()
{
	[ $loglevel -ge 5 ] && \
	printf "$(date "+%m-%d %H:%M:%S.%3N") %7d V %-10s: %b\n" "$$" "${LOG_TAG:0:10}" "$*"
}

function wrapper_run_logf()
{
	"$@" 2>&1 | while read -r msg; do LOGF "$msg"; done
	return ${PIPESTATUS[0]}
}

function wrapper_run_loge()
{
	"$@" 2>&1 | while read -r msg; do LOGE "$msg"; done
	return ${PIPESTATUS[0]}
}

function wrapper_run_logw()
{
	"$@" 2>&1 | while read -r msg; do LOGW "$msg"; done
	return ${PIPESTATUS[0]}
}

function wrapper_run_logi()
{
	"$@" 2>&1 | while read -r msg; do LOGI "$msg"; done
	return ${PIPESTATUS[0]}
}

function wrapper_run_logd()
{
	"$@" 2>&1 | while read -r msg; do LOGD "$msg"; done
	return ${PIPESTATUS[0]}
}

function wrapper_run_logv()
{
	"$@" 2>&1 | while read -r msg; do LOGV "$msg"; done
	return ${PIPESTATUS[0]}
}

# --- Assertions
function assert()
{
	local cond="$1"
	shift
	local msg="$*"

	if ! eval "${cond}"; then
		# fail "ASSERT '${cond}' Failed${msg}"
		LOGF "ERROR: ${msg}"
	fi
	true
}

function assert_success()
{
	assert "[ $? -eq 0 ]" $*
}

function assert_not_empty()
{
	local var_name=$1
	local var_value="$(echo ${!var_name})"
	assert "[ -n \"${var_value}\" ]" "Variable '${var_name}' is empty"
}

function assert_path_exist()
{
	local path="$1"
	shift
	local msg="$*"
	[ -z "$msg" ] && msg="'$path' not exist"
	assert "[ -e \"$path\" ]" "$msg"
}
