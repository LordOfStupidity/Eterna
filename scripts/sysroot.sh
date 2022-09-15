ScriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SystemRoot="$ScriptDirectory/../root"

run() {
    set -x
    "$@"
    { set +x; } 2>/dev/null
}

echo -e "\n\n -> Bootstrapping System Root at $SystemRoot\n\n"
run mkdir -p "$SystemRoot"
cd $ScriptDirectory
run cp -r ../base/
run cd $ScriptDirectory