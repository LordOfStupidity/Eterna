#!/bin/bash

ScriptDirectory="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd )"
BootloaderEFI="$ScriptDirectory/../boot/boot.efi"
KernelDirectory="$ScriptDirectory/../kernel"
BuildDirectory="$KernelDirectory/bin"

run() {
    set -x
    "$@"
    { set +x; } 2>/dev/null
}

# Ensure existence of files used.
if [ ! -f "$BootloaderEFI" ]; then
    echo -e "\n\n -> ERROR: Could not locate bootloader executable at $BootloaderEFI\n\n"
    exit 1
fi
if [ ! -f "$BuildDirectory/kernel.elf" ]; then
    echo -e "\n\n -> ERROR: Could not locate kernel executable at $BuildDirectory/kernel.elf\n\n"
    exit 1
fi
if [ ! -f "$BuildDirectory/dfltfont.psf" ]; then
    echo -e "\n\n -> ERROR: Could not locate font at $BuildDirectory/dfltfont.psf\n\n"
    exit 1
fi
if [ ! -f "$ScriptDirectory/startup.nsh" ]; then
    echo -e "\n\n -> ERROR: Could not locate UEFI startup script at $ScriptDirectory/startup.nsh\n\n"
    exit 1
fi

run mkdir -p $BuildDirectory
run dd if=/dev/zero of=$BuildDirectory/Eterna.img count=93750
run mformat -i $BuildDirectory/Eterna.img -F -v "EFI System" ::
echo -e "\n\n -> Created FAT32 UEFI bootable disk image\n\n"
run mmd -i $BuildDirectory/Eterna.img ::/EFI
run mmd -i $BuildDirectory/Eterna.img ::/EFI/BOOT
run mmd -i $BuildDirectory/Eterna.img ::/Eterna
echo -e "\n\n -> Directories initialized\n\n"
run mcopy -i $BuildDirectory/Eterna.img $BootloaderEFI ::/EFI/BOOT
run mcopy -i $BuildDirectory/Eterna.img $ScriptDirectory/startup.nsh ::
run mcopy -i $BuildDirectory/Eterna.img $BuildDirectory/kernel.elf ::/Eterna
run mcopy -i $BuildDirectory/Eterna.img $BuildDirectory/dfltfont.psf ::/Eterna
echo -e "\n\n -> Resources copied\n\n"
