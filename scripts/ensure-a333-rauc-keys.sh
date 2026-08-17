#!/bin/sh
set -eu

PROJECT_ROOT="${A333_PROJECT_ROOT:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
KEY_DIR="${A333_RAUC_KEY_DIR:-$PROJECT_ROOT/keys}"
KEY_FILE="${A333_RAUC_KEY:-$KEY_DIR/a333-dev.key.pem}"
CERT_FILE="${A333_RAUC_CERT:-$KEY_DIR/a333-dev.cert.pem}"

mkdir -p "$KEY_DIR"

if [ ! -f "$KEY_FILE" ] || [ ! -f "$CERT_FILE" ]; then
	if [ -n "${A333_RAUC_KEY:-}" ] || [ -n "${A333_RAUC_CERT:-}" ]; then
		echo "RAUC key and certificate must both exist when A333_RAUC_KEY/A333_RAUC_CERT are used" >&2
		exit 1
	fi

	echo "RAUC: generating development signing key in $KEY_DIR" >&2
	openssl req -x509 -newkey rsa:3072 -nodes \
		-keyout "$KEY_FILE" \
		-out "$CERT_FILE" \
		-sha256 -days 3650 \
		-subj "/CN=Allwinner A333 Buildroot development update key/" \
		-addext "basicConstraints=critical,CA:TRUE" \
		-addext "keyUsage=critical,digitalSignature,keyCertSign,cRLSign" \
		-addext "extendedKeyUsage=critical,codeSigning"
	chmod 0600 "$KEY_FILE"
	chmod 0644 "$CERT_FILE"
fi

printf '%s\n%s\n' "$KEY_FILE" "$CERT_FILE"
