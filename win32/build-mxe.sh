#!/bin/sh
cd /opt
git clone --depth 1 https://github.com/mxe/mxe.git
cd mxe/
rm -rf .git* .travis.yml
make cc MXE_PLUGIN_DIRS=/opt/mxe/plugins/gcc8 MXE_TARGETS=x86_64-w64-mingw32.shared MXE_USE_CCACHE= --jobs=4 JOBS=2
make clean-junk
