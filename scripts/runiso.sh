ScriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BuildDirectory="$ScriptDirectory/../kernel/bin"
OVMFDirectory="$ScriptDirectory/../OVMFbin"
if ! [ -f $BuildDirectory/Eterna.iso ] ; then
	echo -e "\n\n -> ERROR: Couldn't locate $BuildDirectory/Eterna.iso\n\n"
	exit 1
fi
if ! [ -f $OVMFDirectory/OVMF_CODE-pure-efi.fd ] ; then
	echo -e "\n\n -> ERROR: Couldn't locate $OVMFDirectory/OVMF_CODE-pure-efi.fd\n\n"
	exit 1
fi
if ! [ -f $OVMFDirectory/OVMF_VARS_Eterna.fd ] ; then
	if ! [ -f $OVMFDirectory/OVMF_VARS-pure-efi.fd ] ; then
		echo -e "\n\n -> ERROR: Couldn't locate $OVMFDirectory/OVMF_VARS-pure-efi.fd\n\n"
		exit 1
	fi
	cp $OVMFDirectory/OVMF_VARS-pure-efi.fd $OVMFDirectory/OVMF_VARS_Eterna.fd
fi
qemu-system-x86_64 \
    -d cpu_reset \
    -machine q35 \
    -cpu qemu64 \
    -m 100M \
    -soundhw pcspk \
	-net none \
    -rtc base=localtime,clock=host,driftfix=none \
    -drive format=raw,file=$BuildDirectory/Eterna.iso,media=cdrom \
    -drive if=pflash,format=raw,unit=0,file=$OVMFDirectory/OVMF_CODE-pure-efi.fd,readonly=on \
    -drive if=pflash,format=raw,unit=1,file=$OVMFDirectory/OVMF_VARS_Eterna.fd