#!/bin/sh

mkdir build
cd build
cmake -G "Unix Makefiles" -DTARGET_PLATFORM=$1 -DPY_VERSION=$2 -DPython3_ROOT_DIR=$3 -DPython_ROOT_DIR=$3 -DPython_EXECUTABLE=$4 -DCMAKE_INSTALL_PREFIX=../pyomvbb/omvbb ..
cmake --build . --target install
cd ..

