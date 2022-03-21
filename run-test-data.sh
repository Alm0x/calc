#!/bin/bash
set -euo pipefail
[ "$#" -ge 1 ] || (echo "Expected at least one argument: command to run" && exit 1)

echo "test-data/01..."
"$@" test-data/01.in test-data/01.out
diff -bB test-data/01.out test-data/01.expected
echo "    ok"

echo "test-data/02..."
./test-data/02.gen.py >./test-data/02.in
"$@" test-data/02.in test-data/02.out
diff -bB test-data/02.out test-data/02.expected
echo "    ok"

echo "Bad input..."
set +e
"$@" test-data/non-existing.in test-data/bad.out 2>test-data/bad-err.out
RET_CODE=$?
set -e
[ $RET_CODE == 1 ]
! [ -s .out.tmp ]
echo "    checking stderr..."
diff -bB test-data/bad-err.out <(echo "Unable to open 'test-data/non-existing.in' for reading")
echo "    ok"

echo "All ok."
