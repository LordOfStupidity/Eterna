cmake_minimum_required(VERSION 3.20)
set(ETERNA_VERSION 0.0.1)
set(ETERNA_LANGUAGES ASM_NASM CXX C)

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH REPO_DIR)

include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")

# use clang toolchain by default
set(CMAKE_TOOLCHAIN_FILE toolchain.cmake)

# use `ccache` if it is installed in system's path
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# Export compilation database in JSON format
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# Output executable files to '/bin' directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

project(ETERNA VERSION ${ETERNA_VERSION} LANGUAGES ${ETERNA_LANGUAGES})

# Interrupts must be compiled with general registers only.
add_library(Interrupts interrupts/interrupts.cc panic.cc)

target_compile_options(Interrupts PRIVATE -ffreestanding -fno-exceptions -mcmodel=kernel -mgeneral-regs-only -mno-red-zone -c)
target_include_directories(Interrupts PRIVATE ${REPO_DIR}/include)

add_library(Assembly arch/${ARCH}/gdt.asm interrupts/syscalls.asm)
string(APPEND CMAKE_ASM_NASM_COMPILE_OBJECT " -f elf64")

add_executable(Kernel kernel.cc)
set_target_properties(Kernel PROPERTIES OUTPUT_NAME kernel.elf)
target_compile_definitions(Kernel PRIVATE ${MACHINE} ${ARCH})
if(HIDE_UART_COLOR_CODES)
    target_compile_definitions(Kernel PRIVATE "UART_HIDE_COLOR_CODES")
endif()
target_compile_options(Kernel PRIVATE -ffreestanding -fno-exceptions -fno-rtti -fshort-wchar -mcmodel=kernel -mno-red-zone -Wall -Wextra -Werror)
target_link_options(Kernel PRIVATE -T ${CMAKE_CURRENT_LIST_DIR}/arch/${ARCH}/kernel.ld -static -Bsymbolic -nostdlib -z max-page-size=0x1000)
target_include_directories(Kernel PRIVATE ${REPO_DIR}/include/)
target_link_libraries(Kernel PRIVATE Interrupts)
target_link_libraries(Kernel PRIVATE Assembly)

# Ensure `kernel/bin` directory exists and copy `resources` into it
add_custom_command(TARGET Kernel PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_LIST_DIR}/bin COMMAND ${CMAKE_COMMAND} -E copy_directory ${REPO_DIR}/resources ${CMAKE_CURRENT_LIST_DIR}/bin VERBATIM)

set(SCRIPTS_DIR "${REPO_DIR}/scripts")
cmake_path(NATIVE_PATH SCRIPTS_DIR NORMALIZE SCRIPTS_DIR)
set(IMAGE_DIR "${REPO_DIR}/kernel/bin")
cmake_path(NATIVE_PATH IMAGE_DIR NORMALIZE IMAGE_DIR)

# Boot media generation: raw FAT32 Image.
find_program(DD_PROGRAM dd)
find_program(MTOOLS_PROGRAM mtools)
if(DD_PROGRAM AND MTOOLS_PROGRAM)
    message(VERBOSE "Found dd and mtools, creating target: image_raw")
    add_custom_target(
        image_raw
        COMMAND ${CMAKE_COMMAND} -E make_directory ${IMAGE_DIR}
        COMMAND ${DD_PROGRAM} if=/dev/zero of=${IMAGE_DIR}/Eterna.img count=93750
        COMMAND mformat -i ${IMAGE_DIR}/Eterna.img -F -v "EFI System" ::
        COMMAND mmd -i ${IMAGE_DIR}/Eterna.img ::/EFI
        COMMAND mmd -i ${IMAGE_DIR}/Eterna.img ::/EFI/Boot
        COMMAND mmd -i ${IMAGE_DIR}/Eterna.img ::/Eterna
        ${REPO_DIR}/boot/boot.efi ::/EFI/BOOT
        COMMAND mcopy -i ${IMAGE_DIR}/Eterna.img ${SCRIPTS_DIR}/startup.nsh ::
        COMMAND mcopy -i ${IMAGE_DIR}/Eterna.img ${IMAGE_DIR}/kernel.elf ::/Eterna
        COMMAND mcopy -i ${IMAGE_DIR}/Eterna.img ${IMAGE_DIR}/*.psf ::/Eterna
        COMMENT "Generating UEFI-compatible FAT32 boot media"
        DEPENDS
        ${REPO_DIR}/boot/main.efi
        ${SCRIPTS_DIR}/startup.nsh
        USES_TERMINAL
        VERBATIM
    )
    # Image generation relies on kernel.elf
    add_dependencies(image_raw Kernel)
else()
    message("-- dd and mtools not found on system, skipping image_raw target generation")
endif()

