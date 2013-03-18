VERSION = 2.6.39.2

install:
	sudo cp -p linux-${VERSION}/arch/arm/boot/uImage /srv/tftp
	sudo cp -p rootfs/boot/uInitrd /srv/tftp

all: uImage uInitrd

uImage:
	sudo cp -p linux-${VERSION}/arch/arm/boot/uImage ./rootfs/boot 

uInitrd: rootfs/usr/bin/qemu-arm-static rootfs/boot/mkinitrd.sh
	chroot rootfs /boot/mkinitrd.sh 

#	sudo debootstrap --verbose --foreign --arch=armel --variant=minbase --include=module-init-tools,udev,vim-tiny,bzip2,devio,mtd-utils	squeeze rootfs http://ftp.jp.debian.org/debian

rootfs: 
	sudo debootstrap --verbose --foreign --arch=armel --variant=minbase --include=module-init-tools,udev,vim-tiny,bzip2,devio,mtd-utils,initramfs-tools,uboot-mkimage	squeeze rootfs http://ftp.jp.debian.org/debian
	sudo cp -p /usr/bin/qemu-arm-static ./rootfs/usr/bin
	sudo chroot rootfs ./debootstrap/debootstrap --second-stage

rootfs/usr/bin/qemu-arm-static: /usr/bin/qemu-arm-static
	sudo cp -p /usr/bin/qemu-arm-static ./rootfs/usr/bin

mkinitrd: rootfs/boot/config-${VERSION}
	echo "mkinitramfs -o initrd.img-${VERSION} ${VERSION}" > mkinitrd.sh 
	echo "mkimage -A arm -O linux -T ramdisk -C none -a 0 -e 0 -n initramfs \
 -d ./initrd.img-${VERSION} ./uInitrd" >> mkinitrd.sh 
	sudo cp -p mkinitrd.sh rootfs/boot
	echo "run command: chroot rootfs; cd boot; ./mkinitrd.sh"

rootfs/boot/config-${VERSION}:
	sudo cp -p linux-${VERSION}/.config rootfs/boot/config-${VERSION}
