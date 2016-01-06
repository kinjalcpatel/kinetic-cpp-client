#!/bin/bash

DIR="./bin"

echo "Linting files"
find ./{src,include} \( -name "*.h" -o -name "*.cc" \) -type f | grep -v pb | xargs python $DIR/cpplint.py --root=src --header-guard-prefix=KINETIC_CPP_CLIENT --filter=-legal/copyright,-build/include,-whitespace/comments,-readability/streams,-runtime/references,-readability/casting,-runtime/arrays,-runtime/printf
