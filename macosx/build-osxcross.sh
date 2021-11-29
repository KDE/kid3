#!/bin/bash

thisdir=$(cd $(dirname $0); pwd)

# sudo aptitude install libxml2-dev llvm clang

set -e

# Build xar
test -d xar || git clone https://github.com/mackyle/xar
if ! test -f xar-1.6.1.tgz; then
  cd xar
  wget https://patch-diff.githubusercontent.com/raw/mackyle/xar/pull/23.patch
  patch -p1 -i 23.patch
  cd xar
  ./autogen.sh
  ./configure --prefix=/opt/osxcross/target
  make
  make install DESTDIR=$(pwd)/inst
  cd inst
  tar czf ../../../xar-1.6.1.tgz opt
  cd ../../..
fi

# libdmg-hfsplus
test -d libdmg-hfsplus || git clone https://github.com/vitamin-caig/libdmg-hfsplus
if ! test -f libdmg-hfsplus-20190605.tgz; then
  cd libdmg-hfsplus
  cmake -DCMAKE_INSTALL_PREFIX=/opt/osxcross/target .
  make
  make install DESTDIR=$(pwd)/inst
  cd inst
  tar czf ../../libdmg-hfsplus-20190605.tgz opt
  cd ../..
fi

# build osxcross
test -d osxcross || git clone https://github.com/tpoechtrager/osxcross
cd osxcross
test -d apple-libtapi || git clone https://github.com/tpoechtrager/apple-libtapi.git
test -d cctools-port || git clone https://github.com/tpoechtrager/cctools-port.git
cd ..
if ! test -d /opt/osxcross; then
  sudo cp -a osxcross /opt/
  sudo tar xzf xar-1.6.1.tgz -C /
  sudo tar xzf libdmg-hfsplus-20190605.tgz -C /
  sudo chown -R $USER:$USER /opt/osxcross
fi

# Extract MacOSX11.1.sdk.tar.xz, add a symlink to it from /opt/osxcross/tarballs/
test -f $thisdir/MacOSX11.1.sdk.tar.xz || wget https://github.com/joseluisq/macosx-sdks/releases/download/11.1/MacOSX11.1.sdk.tar.xz -O $thisdir/MacOSX11.1.sdk.tar.xz
cd /opt/osxcross/tarballs
ln -sf $thisdir/MacOSX11.1.sdk.tar.xz
cd ..

export PATH=/opt/osxcross/target/bin:$PATH

if ! test -f /opt/osxcross/target/bin/x86_64-apple-darwin20.2-machocheck; then
  UNATTENDED=1 ./build.sh
fi

if ! test -f /opt/osxcross/target/include/tapi/Defines.h; then
  cd apple-libtapi
  INSTALLPREFIX=/opt/osxcross/target ./build.sh && ./install.sh
  cd ..
fi

if ! test -f /opt/osxcross/target/bin/x86_64-apple-darwin20.2-inout; then
  cd cctools-port/cctools/
  ./configure --prefix=/opt/osxcross/target --target=x86_64-apple-darwin20.2 --with-libtapi=/opt/osxcross/target
  make && make install
  cd ../..
fi

if ! test -f /opt/osxcross/target/bin/osxcross-llvm-dsymutil; then
  ./build_llvm_dsymutil.sh
fi

if ! test -f /opt/osxcross/build/compiler-rt/build/lib/darwin/libclang_rt.tsan_osx_dynamic.dylib; then
  ./build_compiler_rt.sh
  LLVM_VERSION=$(llvm-config --version) # Debian 11: 11.0.1

  mkdir -p /opt/osxcross/target/lib/clang/${LLVM_VERSION}/lib/darwin
  cp -rv /opt/osxcross/build/compiler-rt/compiler-rt/include/sanitizer /opt/osxcross/target/lib/clang/${LLVM_VERSION}/include
  cp -v /opt/osxcross/build/compiler-rt/compiler-rt/build/lib/darwin/*.a /opt/osxcross/target/lib/clang/${LLVM_VERSION}/lib/darwin
  cp -v /opt/osxcross/build/compiler-rt/compiler-rt/build/lib/darwin/*.dylib /opt/osxcross/target/lib/clang/${LLVM_VERSION}/lib/darwin
fi

if ! test -f /opt/osxcross/target/bin/otool; then
  ln -s x86_64-apple-darwin20.2-otool target/bin/otool
  ln -s x86_64-apple-darwin20.2-install_name_tool target/bin/install_name_tool
fi

if ! test -d /opt/osxcross/target/lib/ccache/bin; then
  mkdir -p /opt/osxcross/target/lib/ccache/bin/
  for arch in i386 x86_64 x86_64h; do
    for cmd in clang clang++; do
      ln -s /usr/bin/ccache /opt/osxcross/target/lib/ccache/bin/$arch-apple-darwin20.2-$cmd
    done
  done
fi

if test -d /opt/osxcross/build; then
  # Move everything except target out of /opt/osxcross/
  test -d $thisdir/osxcross.trash && rm -rf $thisdir/osxcross.trash
  mv $thisdir/osxcross $thisdir/osxcross.trash
  mkdir $thisdir/osxcross
  for f in *; do test "$f" = "target" || mv "$f" $thisdir/osxcross/; done
  test -d .git && mv .git $thisdir/osxcross/
  mv .gitignore $thisdir/osxcross/
  sudo chown -R root:root /opt/osxcross
fi
