#!/bin/bash
set -e

TOP_DIR="$(pwd)"

sudo apt-get update

. ./ci/travis/lib.sh

build_cppcheck() {
    export CPPCHECK_OPTIONS="-i ./projects/ADuCM3029_ArrowConnect_Greenhouse/src"
    . ./build/cppcheck.sh 
}

build_astyle() {
    export ASTYLE_EXT_LIST=".cpp .hpp"	
    . ./build/astyle.sh
}

build_${BUILD_TYPE}
