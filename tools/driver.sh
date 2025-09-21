#!/bin/bash
# Usage:
#   ./driver.sh source1 [source2 ...] [-o output]
#
# Examples:
#   ./driver.sh foo.tig                 # builds a.out
#   ./driver.sh foo.tig -o foo          # builds foo
#   ./driver.sh foo.tig bar.tig         # builds foo and bar (two executables)

set -e  # stop on error

SCRIPT_PATH="$(realpath "${BASH_SOURCE[0]}")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
COMPILER=$SCRIPT_DIR/../bin/tigerc
RUNTIME_DIR=$SCRIPT_DIR/../lib

if [ ! -x $COMPILER ]; then
    echo "Error: tigerc not found or not executable"
    exit 1
fi

if [ $# -lt 1 ]; then
    echo "Usage: $0 <source1> [source2 ...] [-o output]"
    exit 1
fi

OUTPUT=""
SOURCES=()

# Parse args
while [[ $# -gt 0 ]]; do
    case "$1" in
        -o)
            if [ -z "$2" ]; then
                echo "Error: -o requires an argument"
                exit 1
            fi
            OUTPUT="$2"
            shift 2
            ;;
        *)
            SOURCES+=("$1")
            shift
            ;;
    esac
done

# Validate -o
if [ -n "$OUTPUT" ] && [ "${#SOURCES[@]}" -ne 1 ]; then
    echo "Error: -o requires exactly one source file"
    exit 1
fi

# Build loop
for SRC in "${SOURCES[@]}"; do
    
    SRC_NAME=$(basename "$SRC")
    ASM_FILE="${SRC_NAME%.*}.s"
    OBJ_FILE="${SRC_NAME%.*}.o"

    if [ -n "$OUTPUT" ]; then
        EXE="$OUTPUT"
    elif [ "${#SOURCES[@]}" -eq 1 ]; then
        EXE="a.out"
    else
        EXE="${SRC%.*}"   # strip extension
    fi

    echo "=== Building $EXE from $SRC ==="
    
    $COMPILER "$SRC" -o "$ASM_FILE"
    gcc -c "$ASM_FILE" -o "$OBJ_FILE"
    gcc "$OBJ_FILE" $RUNTIME_DIR/runtime.o -o "$EXE"

    echo "✓ Built $EXE"
    echo
done