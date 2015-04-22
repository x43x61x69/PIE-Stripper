//
// Name:    PIE Stripper
// Version:	0.1
//
// The MIT License (MIT)
//
// Copyright (c) 2015 Zhi-Wei Cai.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the 'Software'), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Compile: gcc -Wall -O2 -o piestrip piestrip.c

#define VERSION "0.1"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>


void printLog(char *errMsg, int errType) {
    char *errTypeString;
    switch (errType) {
        case 0:
            errTypeString = "[INFO]  ";
            break;
        case 1:
            errTypeString = "[ERROR] ";
            break;
        default:
            errTypeString = "[???]   ";
            break;
    }
    printf(" # %s %s\n", errTypeString, errMsg);
}

int stripASLR(struct mach_header *mh, char *addr, size_t size) {
    printLog(" >>> Stripping...", 0);
    if (mh->flags & MH_PIE) {
        mh->flags &= ~MH_PIE;
        msync(addr, size, MS_ASYNC);
        return 1;
    }
    return 0;
}

int stripASLR64(struct mach_header_64 *mh, char *addr, size_t size) {
    printLog(" >>> Stripping PIE...", 0);
    if (mh->flags & MH_PIE) {
        mh->flags &= ~MH_PIE;
        msync(addr, size, MS_ASYNC);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    
    printf(" # ====================================\n");
    printf(" #\n");
    printf(" #         PIE Stripper v%s\n", VERSION);
    printf(" #   Copyright (C) 2015 Zhi-Wei Cai.\n");
    printf(" #\n");
    printf(" # ====================================\n # \n");
    
    if (argc < 2) {
        printf(" #  Usage: %s target_file\n", argv[0]);
        return 0;
    }
    
    char                    *addr       = NULL;
    int                     fd          = open(argv[1], O_RDWR);
    size_t                  size;
    struct stat             stat_buf;
    
    struct fat_arch         *fa;
    struct fat_header       *fh;
    struct mach_header      *mh;
    struct mach_header_64   *mh64;
    
    uint32_t                mm;
    uint32_t                err;
    
    fstat(fd, &stat_buf);
    size    = stat_buf.st_size;
    addr    = mmap(0, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
    mm      = *(uint32_t *)(addr);
    
    
    char *buf = "MH_MAGIC: ";
    asprintf(&buf,"%s%04X", buf, mm);
    
    switch(mm) {
        case MH_MAGIC:
            asprintf(&buf,"%s (%s)", buf, "i386");
            printLog(buf, 0);
            mh      = (struct mach_header *)addr;
            addr    += sizeof(struct mach_header);
            err     = stripASLR(mh, addr, size);
            break;
        case MH_MAGIC_64:
            asprintf(&buf,"%s (%s)", buf, "x86_64");
            printLog(buf, 0);
            mh64    = (struct mach_header_64 *)addr;
            addr    += sizeof(struct mach_header_64);
            err     = stripASLR64(mh64, addr, size);
            break;
        case FAT_CIGAM:
            asprintf(&buf,"%s (%s)", buf, "FAT");
            printLog(buf, 0);
            fh                    = (struct fat_header *)addr;
            // fat_arch struct were stored in big-endian.
            uint32_t nfat_arch    = OSSwapBigToHostInt32(fh->nfat_arch);
            fa                    = (struct fat_arch *)(addr + sizeof(struct fat_header));
            for(;nfat_arch-- > 0; fa++) {
                uint32_t offset, cputype;
                cputype     = OSSwapBigToHostInt32(fa->cputype);
                offset      = OSSwapBigToHostInt32(fa->offset);
                char *addr  = NULL;
                switch(cputype) {
                    case 0x7:
                        printLog(" >>> FAT -> i386", 0);
                        addr    = mmap(0, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
                        mh      = (struct mach_header *)(addr + offset);
                        addr    += sizeof(struct mach_header) + offset;
                        err     = stripASLR(mh, addr, size);
                        break;
                    case 0x1000007:
                        printLog(" >>> FAT -> x86_64", 0);
                        addr    = mmap(0, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
                        mh64    = (struct mach_header_64 *)(addr + offset);
                        addr    += sizeof(struct mach_header_64) + offset;
                        err     = stripASLR64(mh64, addr, size);
                        break;
                    default:
                        printLog("UNKNOWN FAT MACH-O TYPE", 1);
                        return 0;
                }
            }
            break;
        default:
            printLog("UNKNOWN MACH-O TYPE", 1);
            return 0;
    }
    
    if (!err) {
        printLog("MH_PIE:   0 (NO ASLR/PIE)", 1);
    }
    
    munmap(addr, size);
    close(fd);
    
    printLog("All done.\n # ", 0);
    printf(" # ====================================\n\n");
    
    return 0;
}