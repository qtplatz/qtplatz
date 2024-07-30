#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/config.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh

__nproc nproc
arch=`uname`-`arch`

function openssl_download {
    OPENSSL_VERSION=$1
	OPENSSL_SOURCE_DIR=$2

    if [ ! -f ${DOWNLOADS}/openssl-${OPENSSL_VERSION}.tar.gz ]; then
		echo "=============================="
		URL="https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/openssl-${OPENSSL_VERSION}.tar.gz"

		echo curl -L -o ${DOWNLOADS}/openssl-${OPENSSL_VERSION}.tar.gz ${URL}
		curl -L -o ${DOWNLOADS}/openssl-${OPENSSL_VERSION}.tar.gz ${URL}
	fi
	if [ -d ${OPENSSL_SOURCE_DIR} ]; then
		echo "OpenSSL source directory: ${OPENSSL_SOURCE_DIR} exits, using it;"
	else
		tar xvf ${DOWNLOADS}/openssl-${OPENSSL_VERSION}.tar.gz -C $(dirname ${OPENSSL_SOURCE_DIR})
	fi
}

OPENSSL_SOURCE_DIR=${SRC}/openssl-${OPENSSL_VERSION}
OPENSSL_BUILD_DIR=${BUILD_ROOT}/openssl-${OPENSSL_VERSION}.release

echo "INSTALLING 'openssl' $cross_target:"
echo "	\${OPENSSL_VERSION}        : ${OPENSSL_VERSION}"
if [ -f ${DOWNLOADS}/openssl-${OPENSSL_VERSION}.tar.gz ]; then
	echo "	Found downloaded file in ${DOWNLOADS}/openssl-${OPENSSL_VERSION}.tar.gz"
else
	echo "	\${DOWNLOAD}               : ${DOWNLOADS}"
fi
echo "	\${OPENSSL_INSTALL_PREFIX} : ${OPENSSL_INSTALL_PREFIX}"
echo "	\${BUILD_ROOT}             : ${BUILD_ROOT}"
if [ ! -z ${cross_target} ]; then echo "	\${CROSS_ROOT}           : ${CROSS_ROOT}"; fi
echo "	\${OPENSSL_BUILD_DIR}      : ${OPENSSL_BUILD_DIR}"
if [ -d ${OPENSSL_INSTALL_PREFIX} ]; then
    echo "openssl-${OPENSSL_VERSION} already installed in ${OPENSSL_INSTALL_PREFIX}"
	prompt
fi

__nproc nproc

openssl_download ${OPENSSL_VERSION} ${OPENSSL_SOURCE_DIR}

if [ -d ${OPENSSL_BUILD_DIR} ]; then
	echo "force clean destination files..."
	set -x
	rm -rf ${OPENSSL_BUILD_DIR}
fi

mkdir -p ${OPENSSL_BUILD_DIR}
cd ${OPENSSL_BUILD_DIR}
set +x
config_args=( "--prefix=${OPENSSL_INSTALL_PREFIX}"
			  "--openssldir=openssl shared"
			  )

echo "${OPENSSL_SOURCE_DIR}/Configure ${config_args[@]}"
prompt
${OPENSSL_SOURCE_DIR}/Configure "${config_args[@]}"
if make; then
	sudo make install
fi
