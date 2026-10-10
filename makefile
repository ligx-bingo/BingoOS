all: BingoOS.img

BingoOS.img: boot/BOOTX64.EFI kernel/Kernel.elf
	dd if=/dev/zero of=$@ bs=1M count=64
	mkfs.vfat -F 32 $@
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ boot/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $@ kernel/Kernel.elf ::/Kernel.elf

boot/BOOTX64.EFI:
	$(MAKE) -C boot

kernel/Kernel.elf:
	$(MAKE) -C kernel

clean:
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	rm -f BingoOS.img

run: BingoOS.img
	qemu-system-x86_64 \
	    -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
	    -drive if=pflash,format=raw,file=OVMF_VARS.fd \
	    -drive format=raw,file=BingoOS.img \
	    -net none \
	    -serial stdio

.PHONY: all clean run