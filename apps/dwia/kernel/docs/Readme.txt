DW1000 Setup Guilde on Hikey960
-------------------------------

1. Download Linux source for Hikey960

	- Download linux kernel from linaro-hikey
	  $ git clone https://android.googlesource.com/kernel/hikey-linaro

	- Checkout to 'android-hikey-linaro-4.14' Branch
	  $ git checkout android-hikey-linaro-4.14

	- Set the repo to 'fb1c96e32288af33cde11fe9f75e7704385d5be1' commit ID
	  $ git reset --hard fb1c96e32288af33cde11fe9f75e7704385d5be1

2. Build Linux Kernel for Hikey960
	- To build the linux kernel, Android build set up is needed.
	- Download and build android as mentioned in AOSP for Hikey960.
	- Copy downloaded linux source to ANDROID_TOP_DIR/kernel/
	- Copy 'build_kernel.sh' to ANDROID_TOP_DIR/build_kernel.sh from kernel/code folder.
	- Provide executable permissions to ANDROID_TOP_DIR/build_kernel.sh.
	- Copy 'mkdtimg' to ANDROID_TOP_DIR/kernel/hikey-linaro/mkdtimg
	- Provice executable permissions to ANDROID_TOP_DIR/kernel/hikey-linaro/mkdtimg
	- Change directory to ANDROID_TOP_DIR/kernel/hikey-linaro/
	- Apply patches from kernel/code/patches/ in numerical order using git
	- Change directory to ANDROID_TOP_DIR
	- Run './build_kernel.sh' script

3. Flashing to Hikey960
	- Set Hikey960 in Fastboot mode
	- Flash boot.img and dt.img from Out folder of an Android.

4. Vefify DW1000 driver
	- After Android boot, following things should be created if the driver loaded properly
		- /dev/dw1000
		- /sys/class/input/input0/spi_mode
		- /sys/class/input/input0/spi_freq
		- /sys/class/input/input0/spi_bpw
		- /sys/class/input/input0/spi_transfer
		- /sys/class/input/input0/enable

5. Test DW1000 communication
	- After applying patches, dw1000-test app will be created in ANDROID_TOP_DIR/kernel/hikey-linaro/
	- Copy dw1000-test bin to android using ADB
	- Also connect MDEK with DW1000 to Hikey960
	- Run the app to check whether driver able to get data
	- Usage:
		./dw1000-test 1 </dev/input/event0>
