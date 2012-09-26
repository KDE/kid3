#!/bin/sh
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON ..
make
cpack
