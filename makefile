all: BingoOS.img

run: BingoOS.img
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
		-drive if=pflash,format=raw,file=OVMF_VARS.fd \
		-drive format=raw,file=BingoOS.img \
		-net none -serial stdio

BingoOS.img: boot/BOOTX64.EFI
	dd if=/dev/zero of=$@ bs=1M count=64
	mkfs.vfat -F 32 $@
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ boot/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI

boot/BOOTX64.EFI: boot/main.c
	$(MAKE) -C boot