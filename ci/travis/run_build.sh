#!/bin/bash
set -e

. ./ci/travis/lib.sh

build_default() {
    export CPPCHECK_OPTIONS="-i ./projects/ADuCM3029_ArrowConnect_Greenhouse/src"
    . ./build/cppcheck.sh 
}

build_astyle() {
    . ./build/astyle.sh
}

build_${BUILD_TYPE:-default}
