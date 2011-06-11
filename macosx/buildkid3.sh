#!/bin/sh
cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= -D WITH_TUNEPIMP=OFF -D WITH_MP4V2=ON ../kid3
make
cpack
