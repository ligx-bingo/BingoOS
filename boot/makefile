CC = clang
LD = lld-link
CFLAGS = --target=x86_64-pc-windows-msvc \
		 -ffreestanding \
		 -fno-stack-protector \
		 -mno-red-zone \
		 -Iefi
LDFLAGS = /subsystem:efi_application \
		  /entry:efi_main \
		  /nodefaultlib \
		  /nologo \
		  /fixed:no

SRCS = $(shell find . -name '*.c')
OBJS = $(patsubst %.c, ../build/boot/%.obj, $(SRCS))

all: BOOTX64.EFI

BOOTX64.EFI: $(OBJS)
	$(LD) $(LDFLAGS) -out:$@ $^

../build/boot/%.obj: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@