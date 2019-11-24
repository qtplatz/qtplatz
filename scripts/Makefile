#!make
OS=$(shell lsb_release -si)
VER=$(shell lsb_release -sr)

default:
	@echo "====================================="
	@echo "You are on the ==== " ${OS} ${VER} "===="
	@echo "Check user-config.jam for python version"
	@echo "Check access permission for /usr/local -- You shoud have write permission"
	@echo "====================================="
	@echo "make dependency"
	@echo "make fonts"
	@echo "make mozc"
	@echo "make bower"
	@echo "make boost"
	@echo "=== Don't forget apply boost-nvcc_hpp.patch if using cuda"
	@echo "make qemu"
	@echo "make cross-tools"
	@echo "make boost-armhf"
	@echo "make cmake -- if Debian 9"
	@echo "make rdkit"
	@echo "make qwt"
	@echo "make gnuplot"
	@echo "make rtags -- use apt install rtags on debian 10"
	@echo "make opencv" || "make opencv.debug"


dependency:
	./linux-initial-setup.sh
	./linux-dependency-install.sh

qemu:
	./linux-qemu-setup.sh

mozc:
	./linux-mozc-setup.sh

fonts:
	./linux-fonts-setup.sh

cross-tools:
	./armhf/debian-multiarch-setup.sh

bower:
	./linux-bower-setup.sh

boost:
	./linux-boost-setup.sh

boost-armhf:
	cross_target=armhf ./linux-boost-setup.sh

rdkit:
	./linux-rdkit-setup.sh

qwt:
	./linux-qwt-setup.sh

gnuplot:
	./linux-gnuplot-setup.sh

ffmpeg:
	./linux-ffmpeg-setup.sh

jpeg:
	./linux-jpeg-setup.sh

opencv:
	./linux-opencv-setup.sh

opencv.debug:
	./linux-opencv-setup.sh debug

arrayfire:
	./linux-arrayfire-setup.sh

rtags:
	./linux-rtags-setup.sh

cmake:
	./linux-cmake-setup.sh

world: dependency cross-tools boost boost-armhf cmake rdkit qwt gnuplot opencv rtags
