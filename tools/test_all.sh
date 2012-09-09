#!/bin/bash

test -f spread.hpp || exit 1

for a in */tests/; do
    cd $a
    test.sh || exit 1
    cd -
done
