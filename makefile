all: kernel

.PHONY: iso
iso:
	genisoimage -R								\
				-b boot/grub/stage2_eltorito	\
				-no-emul-boot					\
				-boot-load-size 4				\
				-A os							\
				-input-charset utf8				\
				-quiet							\
				-boot-info-table				\
				-o os.iso						\
				iso

kernel:
	nasm -f elf32 Kernel/loader.s
	ld -T linker.ld -m elf_i386 Kernel/loader.o -o Kernel/bin/kernel.elf