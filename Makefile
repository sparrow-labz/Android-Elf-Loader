# toolchain path
TOOLCHAIN=/home/mint/Documents/android_dev/arm/bin/
# Only choose one of these, depending on your device...
TARGET64=aarch64-linux-android-gcc
#export TARGET=armv7a-linux-androideabi

all:
	# 64 bit loader
	$(TOOLCHAIN)$(TARGET64) -DOS_LINUX -DARCH_ARM64 elf.c main.c -o loader64 -fPIC -pie -g
	$(TOOLCHAIN)$(TARGET64)  dummy.c -o hello64 -g -fPIC -static -ffunction-sections -fdata-sections -Wl,--gc-sections

	# 32 bit loader
	# $(TOOLCHAIN)armv7a-linux-androideabi21-clang -DOS_LINUX -DARCH_ARMEL elf.c main.c -o loader32 -g -fPIC -pie
	# $(TOOLCHAIN)armv7a-linux-androideabi21-clang dummy.c -o hello32 -g -fPIC -pie

	# # x64 loader
	# gcc dummy.c -g -pie -fPIC -o dummy64
	# gcc -DOS_LINUX -DARCH_X86_64 elf.c main.c -o main64 -g -pie -fPIC

	# # x86 loader
	# gcc dummy.c -g -pie -fPIC -o dummy32
	# gcc -DOS_LINUX -DARCH_I686 elf.c main.c -o main32 -g -pie -fPIC

clean:
	rm loader64 hello64 loader32 hello32 main64 dummy64 main32 dummy32

	# 32 bit loader
	# $(TOOLCHAIN)armv7a-linux-androideabi21-clang -DOS_LINUX -DARCH_ARMEL elf.c main.c -o loader32 -g -fPIC -pie 
	# $(TOOLCHAIN)armv7a-linux-androideabi21-clang dummy.c -o hello32 -g -fPIC -pie 

	# # x64 loader
	# gcc dummy.c -g -pie -fPIC -o dummy64
	# gcc -DOS_LINUX -DARCH_X86_64 elf.c main.c -o main64 -g -pie -fPIC

	# # x86 loader
	# gcc dummy.c -g -pie -fPIC -o dummy32
	# gcc -DOS_LINUX -DARCH_I686 elf.c main.c -o main32 -g -pie -fPIC

clean:
	rm loader64 hello64 loader32 hello32 main64 dummy64 main32 dummy32