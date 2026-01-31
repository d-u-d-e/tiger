# This runner does the following:
# for each .tig progam P in the tests/exec folder
# 1. compile P
# 2. run P
# 3. compare expected output with actual output of P

if [ -z "$1" ]; then
    echo "Usage: $0 /path/to/driver"
    exit 1
fi

CC="$1"
if [ ! -x "$CC" ]; then
    echo "Error: compiler '$CC' not found or not executable"
    exit 1
fi

declare -A expected_outputs
expected_outputs=(
    [primes]="2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179 181 191 193 197 199 "
    [hello_world]="hello world!"
    [args]="hello world!"
    [break]="3452345"
    [many_temporaries]="408"
)

shopt -s nullglob
for src in *.tig; do
    name="$(basename "$src")"
    base="${name%.tig}"
    # Compile
    "$CC" "$src" -o "$base" > /dev/null 2>&1
    
    if [ $? -ne 0 ]; then
        echo "❌  Compilation failed for $src"
        continue
    fi
    
    # Run program and capture output
    actual_output="$(./$base)"

    if [[ -v expected_outputs["$base"] ]]; then
        expected_output="${expected_outputs[$base]}"
        if [ "$actual_output" == "$expected_output" ]; then
            echo "✅  $src passed"
        else
            echo "❌  $src failed"
        fi
    else
        echo "⚠️   $src: no expected output"
    fi
done