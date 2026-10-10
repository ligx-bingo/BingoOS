CC = clang
LD = ld
CFLAGS = --target=x86_64-unknown-none \
         -ffreestanding \
         -fno-stack-protector \
         -mno-red-zone \
         -I ../include \
		 -I ../boot/efi
LDFLAGS = -T linker.ld \
          -nostdlib \
          -static

SRCS = $(shell find . -name '*.c' -not -path './efi/*' | sed 's|^\./||')
OBJS = $(patsubst %.c, ../build/kernel/%.o, $(SRCS))

all: Kernel.elf

Kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

../build/kernel/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f Kernel.elf
	rm -rf ../build/kernel

.PHONY: all clean