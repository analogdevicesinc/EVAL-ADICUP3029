#!/bin/bash

set -e

cppcheck --quiet --force --error-exitcode=1 -i ./projects/ADuCM3029_ArrowConnect_Greenhouse/src .
