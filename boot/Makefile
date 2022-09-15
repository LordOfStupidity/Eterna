CC = clang
LD = lld-link

SRC = main.c

INCLUDE = -I../include
CFLAGS = -target x86_64-unknown-windows -fno-stack-protector -ffreestanding -fshort-wchar -mno-red-zone -mno-mmx -mno-sse $(INCLUDE)
CFLAGS += -Wl,-subsystem:efi_application -nostdlib -Wl,-entry:efi_main -shared -Bsymbolic

all: boot.efi

boot.efi: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.efi