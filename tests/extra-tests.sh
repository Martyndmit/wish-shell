#!/usr/bin/env bash
#
# Additional edge-case tests for wish, beyond the official OSTEP suite.
#
# Each case runs ./wish on an input (as a batch file, or on stdin for the
# interactive mode) inside a fresh temporary directory and compares stdout,
# stderr and the exit status with the expected values.
#
# Usage: tests/extra-tests.sh   (or: make test-extra)

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WISH="$ROOT/wish"
ERR=$'An error has occurred\n'

if [[ ! -x "$WISH" ]]; then
    echo "wish executable does not exist (run make first)" >&2
    exit 1
fi

passed=0
failed=0

# mkscript FILE TEXT: creates an executable script that prints TEXT
mkscript() {
    mkdir -p "$(dirname "$1")"
    printf '#!/bin/sh\necho %s\n' "$2" > "$1"
    chmod +x "$1"
}

# run_case MODE NAME SETUP INPUT EXPECTED_STDOUT EXPECTED_STDERR [EXPECTED_STATUS]
#   MODE   batch (input is passed as a batch file) or stdin (interactive mode)
#   SETUP  bash code run in the working directory before wish starts
run_case() {
    local mode=$1 name=$2 setup=$3 input=$4 want_out=$5 want_err=$6 want_rc=${7:-0}
    local tmp rc ok=1

    tmp=$(mktemp -d)
    mkdir "$tmp/work"
    printf '%s' "$input" > "$tmp/input"
    printf '%s' "$want_out" > "$tmp/want_out"
    printf '%s' "$want_err" > "$tmp/want_err"

    (
        cd "$tmp/work" || exit 99
        eval "$setup"
        if [[ $mode == batch ]]; then
            "$WISH" "$tmp/input"
        else
            "$WISH" < "$tmp/input"
        fi
    ) > "$tmp/got_out" 2> "$tmp/got_err"
    rc=$?

    cmp -s "$tmp/want_out" "$tmp/got_out" || ok=0
    cmp -s "$tmp/want_err" "$tmp/got_err" || ok=0
    [[ $rc -eq $want_rc ]] || ok=0

    if (( ok )); then
        printf 'PASS  %s\n' "$name"
        passed=$((passed + 1))
    else
        printf 'FAIL  %s\n' "$name"
        diff -u --label expected-stdout --label actual-stdout "$tmp/want_out" "$tmp/got_out"
        diff -u --label expected-stderr --label actual-stderr "$tmp/want_err" "$tmp/got_err"
        [[ $rc -eq $want_rc ]] || echo "exit status: expected $want_rc, got $rc"
        failed=$((failed + 1))
    fi
    rm -rf "$tmp"
}

batch()       { run_case batch "$@"; }
interactive() { run_case stdin "$@"; }

echo "== whitespace and input format"
batch "tabs and spaces between words" "" \
    $'\t echo \t a\tb \t\n' $'a b\n' ""
batch "CRLF line endings" "" \
    $'echo crlf\r\nexit\r\n' $'crlf\n' ""
batch "last line without a newline" "" \
    'echo last' $'last\n' ""
long_line="echo$(printf ' w%d' $(seq 5000)) > f"
batch "very long line (5000 arguments)" "" \
    "$long_line"$'\nwc -w f\n' $'5000 f\n' ""

echo "== search path"
batch "first matching directory in path wins" \
    'mkscript d1/hello one; mkscript d2/hello two' \
    $'path d2 d1\nhello\npath d1 d2\nhello\n' $'two\none\n' ""
batch "file without execute permission is skipped" \
    'mkscript d1/prog no; chmod -x d1/prog; mkscript d2/prog yes' \
    $'path d1 d2\nprog\n' $'yes\n' ""
batch "directory with the program's name is skipped" \
    'mkdir -p d1/prog; mkscript d2/prog yes' \
    $'path d1 d2\nprog\n' $'yes\n' ""
batch "relative path directories follow cd" \
    'mkscript a/bin/tool tool' \
    $'path bin\ntool\ncd a\ntool\n' $'tool\n' "$ERR"
batch "empty path: programs fail, built-ins still work" \
    'mkdir sub' \
    $'path\nls\ncd sub\npath /bin\nls ..\n' $'sub\n' "$ERR"
batch "path keeps its own copy of the directory names" \
    'mkscript d1/hello hi' \
    $'path d1\nhello\nhello\n' $'hi\nhi\n' ""

echo "== redirection"
batch "'>' truncates an existing file" \
    "printf 'old old old old\n' > f" \
    $'echo new > f\ncat f\n' $'new\n' ""
batch "'>' captures stderr too" "" \
    $'ls /no/such/dir > f\ncat f\n' "$(ls /no/such/dir 2>&1)"$'\n' ""
batch "'>' into a missing directory is an error" "" \
    $'echo hi > nodir/f\necho next\n' $'next\n' "$ERR"
batch "lone '>' is an error" "" \
    $'>\n' "" "$ERR"
batch "built-in with '>' writes its error into the file" "" \
    $'cd /no/such/dir > f\ncat f\n' "$ERR" ""

echo "== parallel commands"
batch "syntax error rejects the whole line" "" \
    $'echo a & echo b >\necho c\n' $'c\n' "$ERR"
batch "unknown program does not stop the others" "" \
    $'nosuchprogram & echo ok\n' $'ok\n' "$ERR"
batch "extra '&' are ignored" "" \
    $'& echo a & &\n' $'a\n' ""
batch "built-ins run in order within the line" \
    'mkdir sub; touch sub/marker' \
    $'cd sub & ls\n' $'marker\n' ""
batch "exit waits for started programs and stops reading" \
    "printf '#!/bin/sh\nsleep 0.5\necho late\n' > late; chmod +x late" \
    $'path .\nlate & exit\necho never\n' $'late\n' ""

echo "== interactive mode"
interactive "prompt is printed before every command" "" \
    $'echo hi\n' $'wish> hi\nwish> ' ""
interactive "exit ends the session" "" \
    $'exit\necho never\n' 'wish> ' ""

echo
echo "passed: $passed, failed: $failed"
(( failed == 0 ))
