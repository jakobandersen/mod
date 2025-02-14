#!/usr/bin/env bash
j=$CPU_COUNT

mkdir src
cd src
tar xzf ../mod-*.tar.gz --strip-components=1
mkdir build
cd build
if [ $(uname -m) = "arm64" ]; then
       ipo=no
       ipoFlags=""
else
       ipo=yes
       ipoFlags="-DCMAKE_MODULE_LINKER_FLAGS='-flto=$j' -DCMAKE_SHARED_LINKER_FLAGS='-flto=$j'"
fi
cmake ../ -DCMAKE_INSTALL_PREFIX=$PREFIX      \
 -DBUILD_DOC=no                               \
 -DBUILD_POST_MOD_FMT=off                     \
 -DCMAKE_BUILD_TYPE=Release                   \
 -DENABLE_IPO=$ipo $ipoFlags                  \
 -DBUILD_TESTING=on
make -j $j
make tests -j $j
make install
ctest -j $j --output-on-failure -E cmake_add_subdirectory
