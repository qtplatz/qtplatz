#!/bin/bash
# This script was generated by ChatGPT on 2025-04-30, according to a chat
# about adding c++20 capability to Debian 10.  Install it as a system default
# compiler using the update-alternatives command while keeping respective
# gcc-8.3 remains.

set -e

# --- Configuration ---
GCC_VERSION="13.2.0"
GCC_ARCHIVE="gcc-${GCC_VERSION}-x86_64-linux.tar.xz"
GCC_URL="https://github.com/brechtsanders/gcc-release/releases/download/v${GCC_VERSION}/${GCC_ARCHIVE}"
INSTALL_DIR="/opt/gcc-13"

echo ">>> Downloading GCC ${GCC_VERSION}..."
wget -O /tmp/${GCC_ARCHIVE} ${GCC_URL}

echo ">>> Extracting to ${INSTALL_DIR}..."
sudo mkdir -p ${INSTALL_DIR}
sudo tar -xf /tmp/${GCC_ARCHIVE} -C ${INSTALL_DIR} --strip-components=1
rm /tmp/${GCC_ARCHIVE}

echo ">>> Registering with update-alternatives..."
sudo update-alternatives --install /usr/bin/gcc gcc ${INSTALL_DIR}/bin/gcc 100
sudo update-alternatives --install /usr/bin/g++ g++ ${INSTALL_DIR}/bin/g++ 100
sudo update-alternatives --install /usr/bin/cc cc ${INSTALL_DIR}/bin/gcc 100
sudo update-alternatives --install /usr/bin/c++ c++ ${INSTALL_DIR}/bin/g++ 100

echo ">>> Setting GCC 13 as default..."
sudo update-alternatives --set gcc ${INSTALL_DIR}/bin/gcc
sudo update-alternatives --set g++ ${INSTALL_DIR}/bin/g++
sudo update-alternatives --set cc ${INSTALL_DIR}/bin/gcc
sudo update-alternatives --set c++ ${INSTALL_DIR}/bin/g++

echo ">>> Done."
echo ">>> GCC version: $(gcc --version | head -n 1)"
