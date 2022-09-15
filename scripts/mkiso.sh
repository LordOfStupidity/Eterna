#!/bin/bash
ScriptDirectory="$(cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd )"
BuildDirectory="$ScriptDirectory/../kernel/bin"

run() {
    set -x
    "$@"
    { set +x; } 2>/dev/null
}

run $ScriptDirectory/mkimg.sh
run mkdir -p $BuildDirectory/iso
run cp \
        $BuildDirectory/Eterna.img \
        ${BuildDirectory}/iso
run xorriso \
        -as mkisofs \
        -R -f   \
        -e Eterna.img \
        -no-emul-boot \
        -o $BuildDirectory/Eterna.iso \
        $BuildDirectory/iso
run rm -r $BuildDirectory/iso