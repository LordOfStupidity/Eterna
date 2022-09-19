set(ARCH "x86_64" CACHE STRING "The CPU architecture that Eterna will run on")
set_property(CACHE ARCH PROPERTY STRINGS "x86_64")

set(MACHINE "QEMU" CACHE STRING "The machine type that Eterna will run on")
set_property(CACHE MACHINE PROPERTY STRINGS "PC" "QEMU" "VBOX" "VMWARE")

option(HIDE_UART_COLOR_CODES "Do not print ANSI terminal color codes to serial output. Particularly useful if the terminal you are using does not support it. ON by default for compatibility reasons." ON)

option(QEMU_DEBUG "Start QEMU with `-S -s` flags, halting startup until a debugger has been attached." OFF)