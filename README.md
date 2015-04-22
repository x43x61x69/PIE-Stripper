#PIE Stripper

[![Author](https://img.shields.io/badge/Author-Zhi--Wei_Cai-red.svg?style=flat-square)](http://vox.vg/)  ![Build](https://img.shields.io/badge/Build-v0.1-green.svg?style=flat-square)  ![License](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)

"**PIE Stripper**" is a *Position Independent Executable*/*Address Space Layout Randomization* (**PIE**/**ASLR**) remover for Mach-O binary.

Currently it supports **i386**, **x86_64** and **FAT** binary.

For more information, please check out the [Technical Q&A QA1788](https://developer.apple.com/library/mac/qa/qa1788/_index.html).

##Compiling

- Compiling the example `aslr` executable with following command:

```shell
gcc -Wall -O2 -o aslr aslr.c
```

- Compiling the **PIE Stripper** executable with following command:

```shell
gcc -Wall -O2 -o piestrip piestrip.c
```

##Usage 

Run the command a few times and see the memory offsets changing:

```shell
toor$ ./aslr

 # ========================
 #  Time:   1429745215
 # ========================
 #  _TEXT_: 0x102f12e70
 #  Global: 0x102f13028
 #  Stack:  0x7fff5cced694
 #  Heap:   0x7fede9504a50
 # ========================

toor$ ./aslr

 # ========================
 #  Time:   1429745217
 # ========================
 #  _TEXT_: 0x106035e70
 #  Global: 0x106036028
 #  Stack:  0x7fff59bca694
 #  Heap:   0x7fdd7a404d00
 # ========================

toor$ ./aslr

 # ========================
 #  Time:   1429745227
 # ========================
 #  _TEXT_: 0x100cb4e70
 #  Global: 0x100cb5028
 #  Stack:  0x7fff5ef4b694
 #  Heap:   0x7fce90504a50
 # ========================
```

Or use `otool -hv aslr` to check it's PIE status:

```shell
aslr:
Mach header
      magic cputype cpusubtype  caps    filetype ncmds sizeofcmds      flags
MH_MAGIC_64  X86_64        ALL LIB64     EXECUTE    16       1376   NOUNDEFS DYLDLINK TWOLEVEL PIE
```

Then use `piestrip` to remove the PIE from `aslr`:

```shell
./piestrip aslr
```

Redo the steps above and see the differences.


##Changelog

- **0.1**：Initial release. 

##License

See the `LICENSE` file.