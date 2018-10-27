#!/bin/bash
nasm Kernel/Bootloader/bootloader.asm -o Kernel/build/bootloader.bin -f bin

nasm Kernel/kernel.asm -o Kernel/bin/KERNELB -f bin

#Programs
echo "Compiling Programs..."
nasm Programs/FileViewer/file_viewer.asm -o Programs/bin/FileViewer/filesee_asm.o -f as86
bcc -ansi -c Programs/FileViewer/file_viewer.c -o Programs/bin/FileViewer/filesee_c.o
ld86 -d -T4000 Programs/bin/FileViewer/filesee_asm.o Programs/bin/FileViewer/filesee_c.o -o Programs/bin/FileViewer/FILESEE

echo "Done."

./FAT8-Tools/build/FAT8 mk os.flp
./FAT8-Tools/build/FAT8 bootcp os.flp Kernel/build/bootloader.bin
./FAT8-Tools/build/FAT8 cp os.flp Programs/bin/FileViewer/FILESEE
./FAT8-Tools/build/FAT8 cp os.flp Kernel/bin/KERNELB
