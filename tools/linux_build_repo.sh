#!/bin/bash

rm -rf Debug
mkdir Debug
cd Debug && cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Exit with status 0
exit 0
