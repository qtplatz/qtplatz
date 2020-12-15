#!/bin/bash
cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${cwd}/scripts/find_qmake.sh

failed_list=()

PYTHON_VERSION=$(python3 -c "import sys; print('{}.{}'.format(*sys.version_info))")

echo "PYTHON_VERSION: ${PYTHON_VERSION}"

#qt5
list_dependency+=('mesa-common-dev'
		  'libglu1-mesa-dev'
		  'freeglut3-dev'
		  'libxml2-dev'
		  'libxslt1-dev'
		  'libgstreamer-plugins-base0.10' )

#boost
list_dependency+=('libbz2-dev')

#python3
list_dependency+=("python${PYTHON_VERSION}-dev" 'python3-pip')

#rtags
list_dependency+=('clang-3.9'
		  'lldb-3.9'
		  'libclang-3.9-dev')

#qt5 from source build
#list_dependency+=('libclang-3.8-dev')

#rdkit
list_dependency+=('libeigen3-dev' ) # 'libschroedinger-maeparser-dev' <-- something wrong

#cmake+opencv
list_dependency+=('libcurl4-openssl-dev'
		  'libhdf5-dev')

#jpeg
list_dependency+=('nasm'
		  'libass-dev'
		  'libfdk-aac-dev')

#ffmpeg
list_dependency+=('lame'
		  'libtheora-dev'
		  'libvorbis-dev'
		  'libvpx-dev'
		  'libopus-dev'
		  'yasm' 'x264'
		  'libx264-dev'
		  'x265'
		  'libx265-dev'
		  'libmp3lame0'
		  'libmp3lame-dev' )
#opencv
list_dependency+=('python-dev'
		  'python-numpy'
		  'libtbb2'
		  'libtbb-dev'
		  'libpng-dev'
		  'libtiff-dev'
		  'libpng12-dev'
		  'libtiff5-dev'
		  'libgtk2.0-dev'
		  'pkg-config'
		  'libavcodec-dev'
		  'libavformat-dev'
		  'libswscale-dev' )

#echo "Install dependency"
list_dependency+=('libfreeimage-dev'
		  'cmake-curses-gui'
		  'libopenblas-dev'
		  'libfftw3-dev'
		  'liblapacke-dev' # OpenBLAS
		  'libglfw3-dev'
		  'libfontconfig1-dev' )

list_dependency+=('nlohmann-json-dev')

#emacs
list_dependency+=('libxpm-dev' 'libgif-dev' 'gnutls-dev')

#sudo apt update

for arg in "${list_dependency[@]}"; do
    echo sudo apt-get install -y "$arg"
    sudo apt-get install -y "$arg" || failed_list+=("$arg")
done

if [ ${#failed_list[@]} -gt 0 ]; then
   echo "Error: Total " ${#failed_list[@]} packages failed to install.
   for arg in "${failed_list[@]}"; do
	echo "	" "$arg"
   done
fi

sudo python3 -m pip install --upgrade pip
python3 -m pip install numpy

## Check if Qt version 5.15.2 or higher, which libxcb-util1 required
## but not provided on Debian 9/10.
if find_qmake QMAKE; then
	QT_VERSION=$($QMAKE -query QT_VERSION)
	if dpkg --compare-versions ${QT_VERSION} ge "5.15.2"; then
		case "`/usr/bin/lsb_release -si`" in
			Debian)
				if dpkg -s libxcb-util1; then
					echo "libxcb-util1 installed -- nothing to be done"
				else
					echo "libxcb-util1 need to be installed"
					echo "run command: sudo dpkg -i ~/Downloads/libxcb-util1_0.4.0-1+b1_amd64.deb"
					(cd ~/Downloads/;
					 wget http://ftp.br.debian.org/debian/pool/main/x/xcb-util/libxcb-util1_0.4.0-1+b1_amd64.deb
					 sudo dpkg -i libxcb-util1_0.4.0-1+b1_amd64.deb
					)
				fi
				;;
		esac
fi
