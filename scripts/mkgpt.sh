#!/bin/bash

ScriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
KernelDirectory="$ScriptDirectory/../kernel"

run(){
    set -x
    "$@"
    { set +x; } 2>/dev/null
}

run $ScriptDirectory/mkimg.sh
run mkgpt -o $KernelDirectory/bin/Eterna.bin --part $KernelDirectory/bin/Eterna.img --type system