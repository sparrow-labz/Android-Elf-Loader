# toolchain path
TOOLCHAIN=/android_dev/arm/bin/
TARGET64=aarch64-linux-android-gcc

all:
	# 64 bit loader
	$(TOOLCHAIN)$(TARGET64) -DOS_LINUX -DARCH_ARM64 elf.c main.c -o loader64 -fPIC -pie -g
	$(TOOLCHAIN)$(TARGET64)  dummy.c -o hello64 -g -fPIC -static -ffunction-sections -fdata-sections -Wl,--gc-sections

	# 32 bit loader
	$(TOOLCHAIN)armv7a-linux-androideabi21-clang -DOS_LINUX -DARCH_ARMEL elf.c main.c -o loader32 -g -fPIC -pie
	$(TOOLCHAIN)armv7a-linux-androideabi21-clang dummy.c -o hello32 -g -fPIC -static -ffunction-sections -fdata-sections -Wl,--gc-sections
clean:
	rm loader64 hello64 loader32 hello32
