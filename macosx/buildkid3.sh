#!/bin/sh
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON ..
# 32-bit:
# cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=OFF -DWITH_APPS=Qt -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON ..
make
cpack
