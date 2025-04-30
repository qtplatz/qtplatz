The memo below is based on a conversation with ChatGPT as of April 30, 2025.  A topic of a chat was about adding C++20 capability to Debian 10.  Install it as the system default compiler using the update-alternatives command, while keeping the respective gcc-8.3 package.

Prerequisites
----

sudo apt update
sudo apt install -y build-essential wget curl libgmp-dev libmpfr-dev libmpc-dev zlib1g-dev flex bison libisl-dev

Download source code
-----

mkdir -p ~/src/gcc-build ~/src/gcc-src ~/tools/gcc-13
cd ~/src/gcc-src
GCC_VERSION=13.2.0
wget https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz
tar -xf gcc-${GCC_VERSION}.tar.xz
cd gcc-${GCC_VERSION}
./contrib/download_prerequisites

Configure
-----
cd ~/src/gcc-build
${HOME}/src/gcc-src/gcc-13.2.0/configure \
  --prefix=/opt/tools/gcc-13 \
  --enable-languages=c,c++ \
  --disable-multilib \
  --enable-libstdcxx-backtrace

Install
-----
sudo make install

update-alternatives
-----

sudo update-alternatives --install /usr/bin/gcc gcc /opt/tools/gcc-13/bin/gcc 100
sudo update-alternatives --install /usr/bin/g++ g++ /opt/tools/gcc-13/bin/g++ 100
sudo update-alternatives --install /usr/bin/cc cc /opt/tools/gcc-13/bin/gcc 100
sudo update-alternatives --install /usr/bin/c++ c++ /opt/tools/gcc-13/bin/g++ 100

sudo update-alternatives --set gcc /opt/tools/gcc-13/bin/gcc
sudo update-alternatives --set g++ /opt/tools/gcc-13/bin/g++

Verify:
-----
gcc --version
g++ -std=c++20 -dM -E - < /dev/null | grep __cplusplus
# Should output: #define __cplusplus 202002L

To Remove GCC-13 gracefully
-----
sudo update-alternatives --remove gcc /opt/tools/gcc-13/bin/gcc
sudo update-alternatives --remove g++ /opt/tools/gcc-13/bin/g++
sudo rm -rf /opt/tools/gcc-13

Add your .profile or .bashrc
---------------
export PATH=/opt/tools/gcc-13/bin:$PATH
export LD_LIBRARY_PATH=/opt/tools/gcc-13/lib64:$LD_LIBRARY_PATH

ld.so.conf
----

echo "/opt/tools/gcc-13/lib64" | sudo tee /etc/ld.so.conf.d/gcc13.conf
sudo ldconfig
