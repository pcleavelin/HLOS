#!/bin/bash

# Kernel
echo "Assembling Kernel"
nasm -f elf32 Kernel/loader.s

echo "Linking"
ld -T linker.ld -m elf_i386 Kernel/loader.o -o Kernel/bin/kernel.elf

