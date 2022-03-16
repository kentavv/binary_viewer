#!/bin/bash

rm -rf cmake-build-debug cmake-build-release cmake-build-relwithdebinfo
mkdir cmake-build-debug cmake-build-release cmake-build-relwithdebinfo

cd cmake-build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cd -

cd cmake-build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cd -

cd cmake-build-relwithdebinfo
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cd -

