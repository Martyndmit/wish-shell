#!/usr/bin/env bash
#
# Runs the official OSTEP test suite for processes-shell against ./wish.
#
# The tests are not copied into this repository: on the first run they are
# fetched from https://github.com/remzi-arpacidusseau/ostep-projects into
# .ostep-projects/. Set OSTEP_DIR to use an existing local clone instead.
#
# Arguments are passed on to the OSTEP tester, e.g.:
#   scripts/run-tests.sh -c        continue after a failed test
#   scripts/run-tests.sh -t 7 -v   run only test 7, verbosely

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OSTEP_DIR="${OSTEP_DIR:-$ROOT/.ostep-projects}"
OSTEP_URL="https://github.com/remzi-arpacidusseau/ostep-projects.git"
SHELL_DIR="$OSTEP_DIR/processes-shell"

if [[ ! -d "$SHELL_DIR/tests" ]]; then
    echo "Fetching the OSTEP test suite into $OSTEP_DIR ..."
    git clone --quiet --depth 1 "$OSTEP_URL" "$OSTEP_DIR"
fi

make -s -C "$ROOT" --no-print-directory wish
cp "$ROOT/wish" "$SHELL_DIR/wish"

# The tests use paths relative to processes-shell/ (tests/1.in, ...).
cd "$SHELL_DIR"
./test-wish.sh "$@"
