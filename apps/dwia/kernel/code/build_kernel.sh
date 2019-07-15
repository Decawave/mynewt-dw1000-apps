#!/bin/bash -e

pr_msg()
{
#	echo -e  "\e[1m\e[7m>>>>>\e[0m $1 \e[21m"
	echo -e  "\e[1;7m>>>>>\e[27m $1 \e[0m"
}


if [ -z "$ANDROID_BUILD_TOP" ]; then
	pr_msg "Setup Android Build Environment"
	source build/envsetup.sh
	lunch hikey960-userdebug
else
	pr_msg "Android Build Environment already Set"
fi

AOSP_ROOT="$ANDROID_BUILD_TOP"
HIKEY_LINUX_DIR="$AOSP_ROOT/kernel/hikey-linaro"
KERNELBINS_DIR="$AOSP_ROOT/device/linaro/hikey-kernel"
OUTDIR="$AOSP_ROOT/out/target/product/hikey960/"
MKDTIMG="$HIKEY_LINUX_DIR/mkdtimg"


# Remove created boot.img and dt.img files
pr_msg "Remove created boot.img and dt.img files..."
if [ -e $OUTDIR/boot.img ]; then
	rm -v $OUTDIR/boot.img
fi
if [ -e $OUTDIR/dt.img ]; then
	rm -v $OUTDIR/dt.img
fi
if [ -e $OUTDIR/kernel ]; then
	rm -v $OUTDIR/kernel
fi

cd $AOSP_ROOT
if [ -z "$ANDROID_JAVA_HOME" ]; then
	pr_msg "Setup Android Build Environment"
	source $AOSP_ROOT/build/envsetup.sh
	lunch hikey960-userdebug
else
	pr_msg "Android Build Environment already Set"
fi

cd $HIKEY_LINUX_DIR
pr_msg "Compiling Hikey Linux Kernel..."
make ARCH=arm64 hikey960_defconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-android- -j8
#make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j8

pr_msg "Copying Kernel Binaries..."
cp -v $HIKEY_LINUX_DIR/arch/arm64/boot/dts/hisilicon/hi3660-hikey960.dtb \
	$KERNELBINS_DIR/hi3660-hikey960.dtb-4.14
cp -v $HIKEY_LINUX_DIR/arch/arm64/boot/Image.gz-dtb  $KERNELBINS_DIR/Image.gz-dtb-hikey960-4.14



cd $AOSP_ROOT
pr_msg "Creating Boot image..."
#make -j24
make bootimage TARGET_KERNEL_USE=4.14 -j24

pr_msg "Creating DTB image..."
$MKDTIMG -d $KERNELBINS_DIR/hi3660-hikey960.dtb-4.14 -s 2048 -c -o ${OUTDIR}/dt.img

pr_msg "Listing image files"
ls -lh  ${OUTDIR}/{kernel,dt.img,boot.img}
