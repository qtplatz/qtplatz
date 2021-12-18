#!make
arch=$(shell uname)-$(shell arch)

ifeq ($(shell uname),Darwin)
	OS=Darwin
	VER=$(shell uname -r)
else
	OS=$(shell lsb_release -si)
	VER=$(shell lsb_release -sr)
endif
DETECTED_OS = ${OS}_${VER}

ifeq (${OS},Darwin)
all: default_darwin
else ifeq (${DETECTED_OS}, Debian_10)
all: default_debian_10
else
all: default
endif

debug:
	@echo "\tarch: " ${arch}
	@echo "\tOS: " ${OS}
	@echo "\tVER: " ${VER}

default_darwin:
	@echo "====================================="
	@echo "We detect mac osx " ${arch} ${VER}
	@echo "Check user-config.jam for python version"
	@echo "====================================="
	@echo "make dependency"
	@echo "make boost, -- ~/user-config.jam will be created if it does not exists."
	@echo "make qwt"
	@echo "make rdkit"
	@echo "make opencv"
	@echo "make bower"
	@echo "You can install"
	@echo "	brew cask install emacs; brew install gnuplot"

default_debian_10:
	@echo "====================================="
	@echo "We detect debian 10 (Buster)"
	@echo "Check user-config.jam for python version"
	@echo "Check access permission for /usr/local -- You shoud have write permission"
	@echo "====================================="
	@echo "make dependency"
	@echo "make boost, check python version in your user-config.jam"
	@echo "make maeparser (depend on boost)"
	@echo "make rdkit     (depend on maeparser,boost)"
	@echo "make qwt"
	@echo "make opencv" || "make opencv.debug"
	@echo "make fonts"
	@echo "make mozc"
	@echo "make bower"
	@echo "make qemu"
	@echo "make cross-tools"
	@echo "make boost-armhf"
	@echo "make qt5-armhf"
	@echo "make gnuplot"
	@echo "make rtags -- in emacs, M-x 'rtags-install' after install clang-11 and libclang-11-dev"
	@echo "make code"
	@echo "make emacs"

default:
	@echo "====================================="
	@echo "You are on the ==== " ${OS} ${VER} "===="
	@echo "Check user-config.jam for python version"
	@echo "Check access permission for /usr/local -- You shoud have write permission"
	@echo "====================================="
	@echo "make dependency"
	@echo "make cmake -- require 3.13"
	@echo "make fonts"
	@echo "make mozc"
	@echo "make bower"
	@echo "make boost"
	@echo "make qwt"
	@echo "make maeparser (depend on boost)"
	@echo "make rdkit     (depend on maeparser,boost)"
	@echo "make opencv" || "make opencv.debug"
	@echo "make qemu"
	@echo "make cross-tools"
	@echo "make boost-armhf"
	@echo "make qt5-armhf"
	@echo "make gnuplot"
	@echo "make rtags -- in emacs, M-x 'rtags-install' after install clang-11 and libclang-11-dev"
	@echo "make code"
	@echo "make Emacs"
	@echo "=== Don't forget apply boost-nvcc_hpp.patch if using cuda"

dependency:
ifeq (${OS},Darwin)
	./darwin-dependency.sh
else
	./linux-initial-setup.sh
	./linux-dependency-install.sh
endif

Emacs:
	./linux-emacs-setup.sh

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

qt5-armhf-dependency:
	./armhf/qt5-dependency.sh

qt5-armhf: qt5-armhf-dependency
	./armhf/qt5-build.sh

maeparser:
	./linux-maeparser-setup.sh clean
	./linux-maeparser-setup.sh

rdkit:
	./linux-rdkit-setup.sh clean
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

code:
	./linux-vscode-setup.sh

world: dependency cross-tools boost boost-armhf cmake rdkit qwt gnuplot opencv rtags
