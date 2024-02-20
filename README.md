Elf loader runs ELF files in memory on Android.

* only works with static elf file so far.

Original code bases:
binary loaders: https://github.com/malisal/loaders
elf injection: https://github.com/ixty/mandibule

TODO:
1) add dynamic linker support
   - loading the bionic linker via the loader causes crash.
   - Research shows loading the bionic linker as an executable might not be possible.
   - dlopen/dlsym after loaded elf is in memory might be solution. 
