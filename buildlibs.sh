#!/bin/bash
# This script can be used to build Kid3 together with static libraries for the
# Windows and Mac versions of Kid3. Linux and BSD users do not need it because
# the libraries can be installed from their repositories, but they can use it
# to generate a self contained package with a minimum of dependencies.
#
# First you have to install the necessary tools:
#
# For Windows:
#
# Install Qt, you should use the MinGW which comes with Qt and add msys2
# to build the libraries. Additional dependencies can be installed using
# Chocolatey, e.g.
# choco install cmake docbook-bundle ninja python3 Wget xsltproc yasm
# Install additional packages in the MSYS2 MinGW 64-bit shell:
# pacman -S git patch make autoconf automake nasm libtool
# Start the msys shell, add Qt and cmake to the path and start this script.
#
# export QTPREFIX=/c/Qt/5.12.8/mingw73_64
# test -z "${PATH##$QTPREFIX*}" ||
# PATH=$QTPREFIX/bin:$QTPREFIX/../../Tools/mingw730_64/bin:$QTPREFIX/../../Tools/mingw730_64/opt/bin:$PROGRAMFILES/CMake/bin:$PATH
# ../kid3/buildlibs.sh
#
# You can also build a Windows version from Linux using the MinGW cross
# compiler.
# COMPILER=cross-mingw QTPREFIX=/path/to/Qt5.6.3-mingw/5.6.3/mingw49_32 ../kid3/buildlibs.sh
#
# For Mac:
#
# Install XCode with the command line tools and Qt. The other build dependencies
# can be installed with Homebrew, for instance:
# brew install cmake ninja autoconf automake libtool xz nasm docbook-xsl
# Then call from a build directory
# QTPREFIX=/path/to/Qt/5.9.7/clang_64 ../kid3/buildlibs.sh
#
# You can also build a macOS version from Linux using the osxcross toolchain.
# COMPILER=cross-macos QTPREFIX=/path/to/Qt5.9.7-mac/5.9.7/clang_64 ../kid3/buildlibs.sh
# or
# COMPILER=cross-macos QTPREFIX=/path/to/Qt5.9.7-mac/5.9.7/clang_64 QTBINARYDIR=/path/to/Qt5.9.7-linux/5.9.7/gcc_64/bin ../kid3/buildlibs.sh
#
# For Android:
#
# Install Qt and a compatible Android SDK and NDK, for example Qt 5.9.7, NDK 10e or Qt 5.12.2, NDK 19c.
# COMPILER=cross-android QTPREFIX=/path/to/Qt/5.9.7/android_armv7 ANDROID_SDK_ROOT=/path/to/sdk ANDROID_NDK_ROOT=/path/to/ndk-bundle ../buildlibs.sh
#
# For Linux:
#
# To build a self-contained Linux package use
# COMPILER=gcc-self-contained QTPREFIX=/path/to/Qt5.9.7-linux/5.9.7/gcc_64 ../kid3/buildlibs.sh
#
# When cross compiling make sure that the host Qt version is not larger than
# the target Qt version, otherwise moc and plugins will fail. To provide
# host Qt binaries of a suitable version, set the QTBINARYDIR environment
# variable.
#
# The source code for the libraries is downloaded from Debian and Ubuntu
# repositories. If the files are no longer available, use a later version,
# it should still work.
#
# buildlibs.sh will download, build and install zlib, libogg, libvorbis,
# flac, id3lib, taglib, ffmpeg, chromaprint, mp4v2. When the libraries
# are built, the Kid3 package is built. It is also possible to build only
# the libraries or only the Kid3 package.
#
# ../kid3/buildlibs.sh libs
# ../kid3/buildlibs.sh package

# Exit if an error occurs
set -e
shopt -s extglob

thisdir=$(pwd)
srcdir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

kernel=$(uname)
test ${kernel:0:5} = "MINGW" && kernel="MINGW"

# Administrative subtasks

# Changes version and date strings in all known Kid3 files.
if test "$1" = "changeversion"; then
  OLDVER=$2
  NEWVER=$3
  if test -z "$OLDVER" || test -z "$NEWVER"; then
    echo "Usage: $0 $1 old-version-nr new-version-nr, e.g. $0 $1 0.8 0.9"
    exit 1
  fi

  echo "### Change version and date strings"

  DATE=$(LC_TIME=C date)
  DATE_R=$(date -R)
  DATE_F=$(date +"%F")
  DATE_Y=$(date +"%Y")

  OLDMAJOR=$(echo $OLDVER | cut -f 1 -d .)
  OLDMINOR=$(echo $OLDVER | cut -f 2 -d .)
  OLDPATCH=$(echo $OLDVER | cut -f 3 -d .)
  test -z $OLDPATCH && OLDPATCH=0

  NEWMAJOR=$(echo $NEWVER | cut -f 1 -d .)
  NEWMINOR=$(echo $NEWVER | cut -f 2 -d .)
  NEWPATCH=$(echo $NEWVER | cut -f 3 -d .)
  test -z $NEWPATCH && NEWPATCH=0

  cd "$srcdir"
  sed -i "1 i\
kid3 (${NEWVER}-0) unstable; urgency=low\n\n  * New upstream release.\n\n\
 -- Urs Fleisch <ufleisch@users.sourceforge.net>  ${DATE_R}\n" deb/changelog
  sed -i "s/^<releaseinfo>${OLDVER}<\/releaseinfo>$/<releaseinfo>${NEWVER}<\/releaseinfo>/; s/^<year>[0-9]\+<\/year>$/<year>${DATE_Y}<\/year>/; s/^<date>[0-9-]\+<\/date>$/<date>${DATE_F}<\/date>/" doc/en/index.docbook
  sed -i "s/PROJECTVERSION=\"${OLDVER}\"/PROJECTVERSION=\"${NEWVER}\"/" translations/extract-merge.sh
  sed -i "s/^Version:      ${OLDVER}$/Version:      ${NEWVER}/" kid3.spec
  sed -i "s/^Copyright 2003-[0-9]\+ Urs Fleisch <ufleisch@users.sourceforge.net>$/Copyright 2003-${DATE_Y} Urs Fleisch <ufleisch@users.sourceforge.net>/" deb/copyright
  sed -i "1 i\
${DATE}  Urs Fleisch  <ufleisch@users.sourceforge.net>\n\n\t* Release ${NEWVER}\n" ChangeLog
  sed -i "s/PROJECT_NUMBER         = ${OLDVER}/PROJECT_NUMBER         = ${NEWVER}/" Doxyfile
  sed -i "s/set(CPACK_PACKAGE_VERSION_MAJOR ${OLDMAJOR})/set(CPACK_PACKAGE_VERSION_MAJOR ${NEWMAJOR})/; s/set(CPACK_PACKAGE_VERSION_MINOR ${OLDMINOR})/set(CPACK_PACKAGE_VERSION_MINOR ${NEWMINOR})/; s/set(CPACK_PACKAGE_VERSION_PATCH ${OLDPATCH})/set(CPACK_PACKAGE_VERSION_PATCH ${NEWPATCH})/; s/set(RELEASE_YEAR [0-9]\+)/set(RELEASE_YEAR ${DATE_Y})/" CMakeLists.txt
  if test $OLDVER != $NEWVER; then
    OLDCODE=$(sed -n "s/ \+set(QT_ANDROID_APP_VERSION_CODE \([0-9]\+\))/\1/p" CMakeLists.txt)
    NEWCODE=$[ $OLDCODE + 1 ]
    sed -i "s/\( \+set(QT_ANDROID_APP_VERSION_CODE \)\([0-9]\+\))/\1$NEWCODE)/" CMakeLists.txt
  fi
  sed -i "s,^  <releases>.*,&\n    <release version=\"${NEWVER}\" date=\"${DATE_F}\"/>," src/app/org.kde.kid3.appdata.xml
  cd - >/dev/null
  exit 0
fi # changeversion

if test "$1" = "cleanuppo"; then
  echo "### Clean up .po files"

  for f in $srcdir/translations/po/*/*.po; do
    sed -i "/#, \(fuzzy, \)\?qt-\(plural-\)\?format/ d; /#, kde-format/ d; /^#~ msg/ d" $f
  done
  exit 0
fi # cleanuppo

if test "$1" = "makearchive"; then
  VERSION=$2
  if test -z "$VERSION"; then
    VERSION=$(date +"%Y%m%d")
  fi

  DIR=kid3-$VERSION
  TGZ=$DIR.tar.gz
  if test -e "$TGZ"; then
    echo "$TGZ already exists!"
    exit 1
  fi

  cd $srcdir
  git archive --format=tar --prefix=$DIR/ HEAD | gzip >$thisdir/$TGZ
  cd - >/dev/null
  exit 0
fi # makearchive

# Build a docker image to build binary Kid3 packages.
# The docker image can then be started using "rundocker".
# You can then build all using cd ~/projects/kid3/src/; ./build-all.sh
# You need:
# - Kid3 project checked out in ~/projects/kid3/src/kid3
# Linux:
# - Qt 5.9.7 Linux in ~/Development/Qt5.9.7-linux/5.9.7/gcc_64/
# Windows:
# - MinGW cross compiler packages in ~/Development/MinGW_Packages/
# - Qt 5.6.3 MinGW in ~/Development/Qt5.6.3-mingw/5.6.3/mingw49_32/
# - Qt 5.9.7 MinGW in ~/Development/Qt5.9.7-mingw64/5.9.7/mingw53_64/
# - Qt 5.6.3 Linux in ~/Development/Qt5.6.3-linux/5.6.3/gcc_64
# Mac:
# - Mac cross compiler in ~/Development/osxcross/
# - Qt 5.9.7 Mac in ~/Development/Qt5.9.7-mac/5.9.7/clang_64/
# Android:
# - Android SDK in ~/Development/android/sdk/
# - Android NDK in ~/Development/android/sdk/android-ndk-r19c/
# - Qt 5.12.4 Android in ~/Development/Qt5.12.4-android/5.12.4/android_armv7/
# - Sign key in ~/Development/ufleisch-release-key.keystore
# - Gradle cache in ~/.gradle/
if test "$1" = "makedocker"; then
  if ! test -f build-all.sh; then
    cat >build-all.sh <<"EOF"
#!/bin/bash
cd "$(dirname "${BASH_SOURCE[0]}")"
set -e
(cd linux_build && \
   COMPILER=gcc-self-contained \
   QTPREFIX=$HOME/Development/Qt5.9.7-linux/5.9.7/gcc_64 \
   ../kid3/buildlibs.sh)
(cd mingw32_build && \
   COMPILER=cross-mingw \
   QTPREFIX=$HOME/Development/Qt5.6.3-mingw/5.6.3/mingw49_32 \
   QTBINARYDIR=$HOME/Development/Qt5.6.3-linux/5.6.3/gcc_64/bin \
   ../kid3/buildlibs.sh)
(cd mingw64_build && \
   COMPILER=cross-mingw \
   QTPREFIX=$HOME/Development/Qt5.12.8-mingw64/5.12.8/mingw73_64 \
   ../kid3/buildlibs.sh)
(cd macos_build && \
   COMPILER=cross-macos \
   QTPREFIX=$HOME/Development/Qt5.9.7-mac/5.9.7/clang_64 \
   OSXPREFIX=$HOME/Development/osxcross/target \
   LD_LIBRARY_PATH=$OSXPREFIX/lib \
   ../kid3/buildlibs.sh)
(cd android_build && \
   COMPILER=cross-android \
   QTPREFIX=$HOME/Development/Qt5.12.4-android/5.12.4/android_armv7 \
   JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64 \
   ANDROID_SDK_ROOT=$HOME/Development/android/sdk \
   ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/android-ndk-r19c \
   ../kid3/buildlibs.sh)
EOF
    chmod +x build-all.sh
  fi
  mkdir -p docker_image_context/pkg
  cd docker_image_context
  for f in gcc-mingw-w64-dw2-base_4.9.3-13ubuntu2+14.1_amd64.deb \
           gcc-mingw-w64-dw2-i686_4.9.3-13ubuntu2+14.1_amd64.deb \
           g++-mingw-w64-dw2-i686_4.9.3-13ubuntu2+14.1_amd64.deb \
           libmpfr4_3.1.6-1_amd64.deb; do
    test -e pkg/$f || cp -a $HOME/Development/MinGW_Packages/$f pkg/
  done
  echo "### Build docker image"
  docker build -t ufleisch/kid3dev . -f-<<EOF
FROM ubuntu:18.04
RUN apt-get update && apt-get install -y --no-install-recommends \
devscripts build-essential lintian debhelper extra-cmake-modules \
kio-dev kdoctools-dev qtmultimedia5-dev qtdeclarative5-dev \
qttools5-dev qttools5-dev-tools qtdeclarative5-dev-tools \
qml-module-qtquick2 cmake python libid3-3.8.3-dev libflac++-dev \
libvorbis-dev libtag1-dev libchromaprint-dev libavformat-dev \
libavcodec-dev docbook-xsl pkg-config libreadline-dev xsltproc \
debian-keyring ppa-purge dput-ng python-distro-info sudo curl \
g++-4.8 libcloog-isl4 libisl15 mingw-w64 \
locales ninja-build ccache p7zip-full genisoimage \
clang libssl1.0.0 openjdk-8-jdk-headless nasm lib32z1 chrpath
COPY pkg .
RUN dpkg -i *.deb && rm -f *.deb && \
adduser --quiet --disabled-password --home $HOME --gecos "User" $USER && \
echo "$USER:$USER" | chpasswd && usermod -aG sudo $USER && \
locale-gen en_US.UTF-8 && \
mkdir -p $HOME/projects/kid3 $HOME/Development
USER $USER
CMD bash
EOF
  exit 0
fi

# Run docker image created with "makedocker".
if test "$1" = "rundocker"; then
  echo "### Run docker image"
  shift
  docker run --rm -it \
         -v $HOME/projects/kid3:$HOME/projects/kid3 \
         -v $HOME/.gradle:$HOME/.gradle \
         -v $HOME/.gnupg:$HOME/.gnupg:ro \
         -v $HOME/Development:$HOME/Development:ro ufleisch/kid3dev "$@"
  exit 0
fi

# End of subtasks


if test -f CMakeLists.txt; then
  echo "Do not run this script from the source directory!"
  echo "Start it from a build directory at the same level as the source directory."
  exit 1
fi

target=${*:-libs package}

qt_version=5.9.7
zlib_version=1.2.8
zlib_patchlevel=5
libogg_version=1.3.2
libogg_patchlevel=1
libvorbis_version=1.3.6
libvorbis_patchlevel=2
ffmpeg_version=3.2.14
ffmpeg_patchlevel=1~deb9u1
#libav_version=11.12
#libav_patchlevel=1
libflac_version=1.3.3
libflac_patchlevel=1
id3lib_version=3.8.3
id3lib_patchlevel=16.2
taglib_version=1.11.1
chromaprint_version=1.5.0
chromaprint_patchlevel=1
mp4v2_version=2.0.0
mp4v2_patchlevel=5

# Try to find the configuration from an existing build.
if test -z "$COMPILER"; then
  if test -f mingw.cmake; then
    COMPILER=cross-mingw
    QTPREFIX=$(sed -ne '1 s/set(QT_PREFIX \([^)]\+\))/\1/p' mingw.cmake)
  elif test -f osxcross.cmake; then
    COMPILER=cross-macos
    QTPREFIX=$(sed -ne '1 s/set(QT_PREFIX \([^)]\+\))/\1/p' osxcross.cmake)
  elif test $(ls openssl-*/Setenv-android.sh 2>/dev/null | wc -l) != "0"; then
    COMPILER=cross-android
    test -f kid3/CMakeCache.txt &&
      QTPREFIX=$(sed -ne 's/^QT_QMAKE_EXECUTABLE[^=]*=\(.*\)\/bin\/qmake$/\1/p' kid3/CMakeCache.txt)
  elif test -f taglib-${taglib_version}/taglib.sln; then
    COMPILER=msvc
  elif grep -q "CMAKE_CXX_COMPILER.*g++-4\.8" kid3/CMakeCache.txt 2>/dev/null; then
    COMPILER=gcc-self-contained
    QTPREFIX=$(sed -ne 's/^QT_QMAKE_EXECUTABLE[^=]*=\(.*\)\/bin\/qmake$/\1/p' kid3/CMakeCache.txt)
  elif grep -q "^CMAKE_BUILD_TYPE.*=Debug$" kid3/CMakeCache.txt 2>/dev/null; then
    COMPILER=gcc-debug
    QTPREFIX=$(sed -ne 's/^QT_QMAKE_EXECUTABLE[^=]*=\(.*\)\/bin\/qmake$/\1/p' kid3/CMakeCache.txt)
  fi
fi

compiler=${COMPILER:-gcc}
echo -n "### Building $target with $compiler"
if test -n "$QTPREFIX"; then
  echo -n " using $QTPREFIX"
fi
echo "."

qt_major=${QTPREFIX##*5.}
qt_major=${qt_major%%.*}
if test "$qt_major" -gt 11; then
  # Since Qt 5.12.4, OpenSSL 1.1.1 is supported
  openssl_version=1.1.1g
else
  openssl_version=1.0.2u
fi

if test "$compiler" = "cross-mingw"; then
  if test -n "$QTPREFIX" && test -z "${QTPREFIX%%*64?(/)}"; then
    cross_host="x86_64-w64-mingw32"
  else
    cross_host="i686-w64-mingw32"
  fi
elif test "$compiler" = "cross-macos"; then
  cross_host="x86_64-apple-darwin17"
  osxprefix=${OSXPREFIX:-/opt/osxcross/target}
fi

if [[ $target = *"libs"* ]]; then

if test "$compiler" = "gcc-debug"; then
  export CFLAGS="-fPIC"
  export CXXFLAGS="-fPIC"
  FLAC_BUILD_OPTION="--enable-debug"
  ID3LIB_BUILD_OPTION="--enable-debug=minimum"
  AV_BUILD_OPTION="--enable-debug=3 --enable-pic --extra-ldexeflags=-pie"
  CMAKE_BUILD_OPTION="-DCMAKE_BUILD_TYPE=Debug"
elif test "$compiler" = "gcc-self-contained"; then
  export CC="gcc-4.8"
  export CXX="g++-4.8"
  export CFLAGS="-O2 -fPIC"
  export CXXFLAGS="-O2 -fPIC"
  FLAC_BUILD_OPTION="--enable-debug"
  ID3LIB_BUILD_OPTION="--enable-debug=minimum"
  AV_BUILD_OPTION="--enable-debug=1 --enable-pic --extra-ldexeflags=-pie"
  CMAKE_BUILD_OPTION="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
else
  FLAC_BUILD_OPTION="--enable-debug"
  ID3LIB_BUILD_OPTION="--enable-debug=minimum"
  AV_BUILD_OPTION="--enable-debug=1"
  CMAKE_BUILD_OPTION="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
fi

if ! which cmake >/dev/null; then
  echo "cmake not found."
  return 2>/dev/null
  exit 1
fi

if test $kernel = "MSYS_NT-6.1"; then
  kernel="MINGW"
  CONFIGURE_OPTIONS="--build=x86_64-w64-mingw32 --target=i686-w64-mingw32"
fi
if test $kernel = "MINGW"; then
  # Use mingw from Qt
  if test -n "$QTPREFIX"; then
    test -z "${PATH##$QTPREFIX*}" || PATH=$QTPREFIX/bin:$QTPREFIX/../../Tools/mingw492_32/bin:$QTPREFIX/../../Tools/mingw492_32/opt/bin:$PATH
  else
    echo "QTPREFIX is not set"
    exit 1
  fi
  CMAKE_OPTIONS="-G \"MSYS Makefiles\" -DCMAKE_INSTALL_PREFIX=/usr/local"
  CONFIGURE_OPTIONS+=" --prefix=/usr/local"
elif test $kernel = "Darwin"; then
  CMAKE_OPTIONS="-G \"Unix Makefiles\""
fi

if test "$compiler" = "cross-mingw"; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_TOOLCHAIN_FILE=$thisdir/mingw.cmake"
  CONFIGURE_OPTIONS="--host=${cross_host}"
elif test "$compiler" = "cross-macos"; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_TOOLCHAIN_FILE=$thisdir/osxcross.cmake -DCMAKE_C_FLAGS=\"-O2 -mmacosx-version-min=10.7\" -DCMAKE_CXX_FLAGS=\"-O2 -mmacosx-version-min=10.7 -fvisibility=hidden -fvisibility-inlines-hidden -stdlib=libc++\" -DCMAKE_EXE_LINKER_FLAGS=-stdlib=libc++ -DCMAKE_MODULE_LINKER_FLAGS=-stdlib=libc++ -DCMAKE_SHARED_LINKER_FLAGS=-stdlib=libc++"
  CONFIGURE_OPTIONS="--host=${cross_host}"
  export CC=x86_64-apple-darwin17-clang
  export CXX=x86_64-apple-darwin17-clang++
  export AR=x86_64-apple-darwin17-ar
  export CFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=10.7"
  export CXXFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=10.7 -stdlib=libc++"
  export LDFLAGS="$ARCH_FLAG -mmacosx-version-min=10.7 -stdlib=libc++"
fi

if test $kernel = "MINGW" || test "$compiler" = "cross-mingw"; then
  # Build zlib only for Windows.
  ZLIB_ROOT_PATH="$thisdir/zlib-$zlib_version/inst/usr/local"
  TAGLIB_ZLIB_ROOT_OPTION="-DZLIB_ROOT=${ZLIB_ROOT_PATH}"
  CHROMAPRINT_ZLIB_OPTION="-DEXTRA_LIBS=\"-L${ZLIB_ROOT_PATH}/lib -lz\""
fi

if test $kernel = "Darwin"; then
  ARCH=$(uname -m)
  #ARCH=i386
  if test "$ARCH" = "i386"; then
    # To build a 32-bit Mac OS X version of Kid3 use:
    # cmake -G "Unix Makefiles" -DCMAKE_CXX_FLAGS="-arch i386" -DCMAKE_C_FLAGS="-arch i386" -DCMAKE_EXE_LINKER_FLAGS="-arch i386" -DQT_QMAKE_EXECUTABLE=/usr/local/Trolltech/Qt-${qt_version}-i386/bin/qmake -DCMAKE_BUILD_TYPE=Release -DWITH_FFMPEG=ON -DCMAKE_INSTALL_PREFIX= ../kid3
    # Building multiple architectures needs ARCH_FLAG="-arch i386 -arch x86_64",
    # CONFIGURE_OPTIONS="--disable-dependency-tracking", but it fails with libav.
    ARCH_FLAG="-arch i386"
    export CC=gcc
    export CXX=g++
  else
    ARCH_FLAG="-Xarch_x86_64"
  fi
  if [[ $(sw_vers -productVersion) = 10.1* ]]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_C_FLAGS=\"-O2 $ARCH_FLAG -mmacosx-version-min=10.7\" -DCMAKE_CXX_FLAGS=\"-O2 $ARCH_FLAG -mmacosx-version-min=10.7 -fvisibility=hidden -fvisibility-inlines-hidden -stdlib=libc++\" -DCMAKE_EXE_LINKER_FLAGS=\"$ARCH_FLAG -stdlib=libc++\" -DCMAKE_MODULE_LINKER_FLAGS=\"$ARCH_FLAG -stdlib=libc++\" -DCMAKE_SHARED_LINKER_FLAGS=\"$ARCH_FLAG -stdlib=libc++\""
    export CFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=10.7"
    export CXXFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=10.7 -stdlib=libc++"
    export LDFLAGS="$ARCH_FLAG -mmacosx-version-min=10.7 -stdlib=libc++"
  else
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_C_FLAGS=\"-O2 $ARCH_FLAG -mmacosx-version-min=10.5\" -DCMAKE_CXX_FLAGS=\"-O2 $ARCH_FLAG -mmacosx-version-min=10.5 -fvisibility=hidden -fvisibility-inlines-hidden\""
    export CFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=10.5"
    export CXXFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=10.5"
    export LDFLAGS="$ARCH_FLAG -mmacosx-version-min=10.5"
  fi
fi # Darwin

if which wget >/dev/null; then
  DOWNLOAD=wget
else
  DOWNLOAD="curl -skfLO"
fi

test -d buildroot || mkdir buildroot
BUILDROOT=`pwd`/buildroot/

fixcmakeinst() {
  if test -d inst && test $kernel = "MINGW"; then
    cd inst
    if test -d prg; then
      rm -rf usr
      mv prg/msys usr
      rmdir prg
    elif test -d msys; then
      rm -rf usr
      mv msys/1.0 usr
      rmdir msys
    elif test -d MinGW; then
      mv MinGW usr
    elif test -d msys64; then
      rm -rf usr
      mv msys64/usr .
      rmdir msys64
    elif test -d msys32; then
      rm -rf usr
      mv msys32/usr .
      rmdir msys32
    fi
    cd ..
  fi
}


# Download sources

test -d source || mkdir source
cd source

test -f taglib-${taglib_version}.tar.gz ||
  $DOWNLOAD http://taglib.github.io/releases/taglib-${taglib_version}.tar.gz

if test "$compiler" != "cross-android"; then

  test -f flac_${libflac_version}-${libflac_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/flac/flac_${libflac_version}-${libflac_patchlevel}.debian.tar.xz
  test -f flac_${libflac_version}.orig.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/flac/flac_${libflac_version}.orig.tar.xz

  test -f id3lib3.8.3_${id3lib_version}-${id3lib_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_${id3lib_version}-${id3lib_patchlevel}.debian.tar.xz
  test -f id3lib3.8.3_${id3lib_version}.orig.tar.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_${id3lib_version}.orig.tar.gz

  test -f libogg_${libogg_version}-${libogg_patchlevel}.diff.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_${libogg_version}-${libogg_patchlevel}.diff.gz
  test -f libogg_${libogg_version}.orig.tar.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_${libogg_version}.orig.tar.gz

  test -f libvorbis_${libvorbis_version}-${libvorbis_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_${libvorbis_version}-${libvorbis_patchlevel}.debian.tar.xz
  test -f libvorbis_${libvorbis_version}.orig.tar.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_${libvorbis_version}.orig.tar.gz

  if test -n "$ZLIB_ROOT_PATH"; then
    test -f zlib_${zlib_version}.dfsg-${zlib_patchlevel}.debian.tar.xz ||
      $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_${zlib_version}.dfsg-${zlib_patchlevel}.debian.tar.xz
    test -f zlib_${zlib_version}.dfsg.orig.tar.gz ||
      $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_${zlib_version}.dfsg.orig.tar.gz
  fi

  if test -n "${ffmpeg_version}"; then
    test -f ffmpeg_${ffmpeg_version}.orig.tar.xz ||
      $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/ffmpeg/ffmpeg_${ffmpeg_version}.orig.tar.xz
    test -f ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz ||
      $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/ffmpeg/ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz
    ffmpeg_dir=ffmpeg-${ffmpeg_version}
  else
    if test "${libav_version%.*}" = "0.8"; then
      test -f libav_${libav_version}.orig.tar.gz ||
        $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/liba/libav/libav_${libav_version}.orig.tar.gz
      test -f libav_${libav_version}-${libav_patchlevel}.debian.tar.gz ||
        $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/liba/libav/libav_${libav_version}-${libav_patchlevel}.debian.tar.gz
    else
      test -f libav_${libav_version}.orig.tar.xz ||
        $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/liba/libav/libav_${libav_version}.orig.tar.xz
      test -f libav_${libav_version}-${libav_patchlevel}.debian.tar.xz ||
        $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/liba/libav/libav_${libav_version}-${libav_patchlevel}.debian.tar.xz
      ffmpeg_dir=libav-${libav_version}
    fi
  fi

  test -f chromaprint_${chromaprint_version}.orig.tar.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/c/chromaprint/chromaprint_${chromaprint_version}.orig.tar.gz
  test -f chromaprint_${chromaprint_version}-${chromaprint_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/c/chromaprint/chromaprint_${chromaprint_version}-${chromaprint_patchlevel}.debian.tar.xz

  test -f mp4v2_${mp4v2_version}~dfsg0.orig.tar.bz2 ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/m/mp4v2/mp4v2_${mp4v2_version}~dfsg0.orig.tar.bz2
  test -f mp4v2_${mp4v2_version}~dfsg0-${mp4v2_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/m/mp4v2/mp4v2_${mp4v2_version}~dfsg0-${mp4v2_patchlevel}.debian.tar.xz

fi # !cross-android

if test "$compiler" = "cross-android" || test "$compiler" = "gcc-self-contained" || test "$compiler" = "gcc-debug" \
   || ( ( test "$compiler" = "cross-mingw" || test "$kernel" = "MINGW" ) && test "${openssl_version:0:3}" != "1.0" ); then
  # See http://doc.qt.io/qt-5/opensslsupport.html
  test -f Setenv-android.sh ||
    $DOWNLOAD https://wiki.openssl.org/images/7/70/Setenv-android.sh
  test -f openssl-${openssl_version}.tar.gz ||
    $DOWNLOAD https://www.openssl.org/source/openssl-${openssl_version}.tar.gz
fi

# Create patch files

if test "$compiler" = "cross-mingw" || test "$compiler" = "cross-macos"; then
  if test -n "$QTBINARYDIR"; then
    _qt_bin_dir=$QTBINARYDIR
  else
    for d in $thisdir/qtbase5-dev-tools* /usr/lib/${HOSTTYPE/i686/i386}-linux-gnu/qt5/bin /usr/bin; do
      if test -x $d/moc; then
        _qt_bin_dir=$d
        break
      fi
    done
  fi
  if test -n "$QTPREFIX"; then
    _qt_prefix=$QTPREFIX
  else
    if test "$compiler" = "cross-mingw"; then
      for d in /windows/Qt/${qt_version}/mingw* /windows/Qt/Qt${qt_version}/${qt_version}/mingw* $thisdir/Qt*-mingw/${qt_version}/mingw*; do
        if test -d $d; then
          _qt_prefix=$d
          break
        fi
      done
    elif test "$compiler" = "cross-macos"; then
      for d in $thisdir/Qt*-mac/${qt_version}/clang_64 $HOME/Development/Qt*-mac/${qt_version}/clang_64; do
        if test -d $d; then
          _qt_prefix=$d
          break
        fi
      done
    fi
  fi
fi # cross-mingw || cross-macos
if test "$compiler" = "cross-mingw"; then
  cat >$thisdir/mingw.cmake <<EOF
set(QT_PREFIX ${_qt_prefix})

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER ${cross_host}-gcc)
set(CMAKE_CXX_COMPILER ${cross_host}-g++)
set(CMAKE_RC_COMPILER ${cross_host}-windres)
set(CMAKE_FIND_ROOT_PATH /usr/${cross_host} \${QT_PREFIX} $thisdir/buildroot/usr/local ${ZLIB_ROOT_PATH} $thisdir/$ffmpeg_dir/inst/usr/local)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(QT_BINARY_DIR ${_qt_bin_dir})
set(QT_LIBRARY_DIR  \${QT_PREFIX}/lib)
set(QT_QTCORE_LIBRARY   \${QT_PREFIX}/lib/libQt5Core.a)
set(QT_QTCORE_INCLUDE_DIR \${QT_PREFIX}/include/QtCore)
set(QT_MKSPECS_DIR  \${QT_PREFIX}/mkspecs)
set(QT_MOC_EXECUTABLE  \${QT_BINARY_DIR}/moc)
set(QT_UIC_EXECUTABLE  \${QT_BINARY_DIR}/uic)

foreach (_exe moc rcc lupdate lrelease uic)
  if (NOT TARGET Qt5::\${_exe})
    add_executable(Qt5::\${_exe} IMPORTED)
    set_target_properties(Qt5::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_BINARY_DIR}/\${_exe}
    )
  endif ()
endforeach (_exe)
EOF
elif test "$compiler" = "cross-macos"; then
  test -z ${PATH##$osxprefix/*} || PATH=$osxprefix/bin:$osxprefix/SDK/MacOSX10.13.sdk/usr/bin:$PATH
  cat >$thisdir/osxcross.cmake <<EOF
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif (POLICY CMP0025)

set(QT_PREFIX $_qt_prefix)

set (CMAKE_SYSTEM_NAME Darwin)
set (CMAKE_C_COMPILER $osxprefix/lib/ccache/bin/x86_64-apple-darwin17-clang)
set (CMAKE_CXX_COMPILER $osxprefix/lib/ccache/bin/x86_64-apple-darwin17-clang++)
set (CMAKE_FIND_ROOT_PATH $osxprefix/x86_64-apple-darwin17;$osxprefix/SDK/MacOSX10.13.sdk/usr;$osxprefix/x86_64-apple-darwin17;$osxprefix/SDK/MacOSX10.13.sdk;$osxprefix/SDK/MacOSX10.13.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers;\${QT_PREFIX};$thisdir/buildroot/usr/local)
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_AR:FILEPATH x86_64-apple-darwin17-ar)
set (CMAKE_RANLIB:FILEPATH x86_64-apple-darwin17-ranlib)

set(QT_INCLUDE_DIRS_NO_SYSTEM ON)
set(QT_BINARY_DIR ${_qt_bin_dir})
set(QT_LIBRARY_DIR  \${QT_PREFIX}/lib)
set(Qt5Core_DIR \${QT_PREFIX}/lib/cmake/Qt5Core)
set(QT_MKSPECS_DIR  \${QT_PREFIX}/mkspecs)
set(QT_MOC_EXECUTABLE  \${QT_BINARY_DIR}/moc)
set(QT_UIC_EXECUTABLE  \${QT_BINARY_DIR}/uic)

foreach (_exe moc rcc lupdate lrelease uic)
  if (NOT TARGET Qt5::\${_exe})
    add_executable(Qt5::\${_exe} IMPORTED)
    set_target_properties(Qt5::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_BINARY_DIR}/\${_exe}
    )
  endif ()
endforeach (_exe)
EOF
fi # cross-mingw, cross-macos

test -f fink_flac.patch ||
  cat >fink_flac.patch <<"EOF"
diff -ruN flac-1.2.1/patches/fixrpath.sh flac-1.2.1.new/patches/fixrpath.sh
--- flac-1.2.1/patches/fixrpath.sh	1969-12-31 19:00:00.000000000 -0500
+++ flac-1.2.1.new/patches/fixrpath.sh	2008-02-18 10:51:07.000000000 -0500
@@ -0,0 +1,28 @@
+#!/bin/sh
+# $Id: fixrpath,v 1.1 2004/05/27 10:48:25 kobras Exp $
+# libtool -rpath workaround based on a suggestion by Yann Dirson
+# <dirson@debian.org>
+#
+# It is supposed to be inserted in configure.in, but I didn't want
+# to re-run autoconf (since that bloats the Debian diff unnecessarily),
+# so I just patch libtool after running configure.  -- Richard Braakman
+# <dark@xs4all.nl>
+#
+# The version of libtool included with LessTif unfortunately insists on
+# linking with -rpath, i.e. hardwiring locations. This is not desirable.
+#
+# The dummy define is improbable enough not to conflict with anything; it is
+# just here to fool libtool by making it believe it gave some useful info to
+# gcc.
+#
+# This will also patch the generated libtool to explicitly
+# link libraries against the libraries they depend on.  (particularly libc)
+
+for i in libtool libtool-disable-static; do
+sed < $i > $i-2 \
+	-e 's/^hardcode_libdir_flag_spec.*$/hardcode_libdir_flag_spec=" -D__LIBTOOL_IS_A_FOOL__ "/' \
+	-e '/^archive_cmds="/s/"$/ \$deplibs"/'
+mv $i-2 $i
+chmod 755 $i
+done
+
diff -ruN flac-1.2.1/patches/ltmain.sh.patch flac-1.2.1.new/patches/ltmain.sh.patch
--- flac-1.2.1/patches/ltmain.sh.patch	1969-12-31 19:00:00.000000000 -0500
+++ flac-1.2.1.new/patches/ltmain.sh.patch	2008-02-18 10:48:01.000000000 -0500
@@ -0,0 +1,11 @@
+--- ltmain.sh.orig	Fri Feb  4 21:22:19 2005
++++ ltmain.sh	Wed Feb 23 19:09:37 2005
+@@ -2280,7 +2280,7 @@
+ 	   { test "$prefer_static_libs" = no || test -z "$old_library"; }; then
+ 	  if test "$installed" = no; then
+ 	    notinst_deplibs="$notinst_deplibs $lib"
+-	    need_relink=yes
++	    need_relink=no
+ 	  fi
+ 	  # This is a shared library
+ 
diff -ruN flac-1.2.1/patches/nasm.h.patch flac-1.2.1.new/patches/nasm.h.patch
--- flac-1.2.1/patches/nasm.h.patch	1969-12-31 19:00:00.000000000 -0500
+++ flac-1.2.1.new/patches/nasm.h.patch	2007-01-25 21:34:54.000000000 -0500
@@ -0,0 +1,14 @@
+--- src/libFLAC/ia32/nasm.h~	2005-01-25 13:14:22.000000000 +0900
++++ src/libFLAC/ia32/nasm.h	2006-03-15 18:07:23.000000000 +0900
+@@ -49,6 +49,11 @@
+ 	%idefine code_section section .text align=16
+ 	%idefine data_section section .data align=32
+ 	%idefine bss_section  section .bss  align=32
++%elifdef OBJ_FORMAT_macho
++	%define FLAC__PUBLIC_NEEDS_UNDERSCORE
++	%idefine code_section section .text
++	%idefine data_section section .data
++	%idefine bss_section  section .bss
+ %else
+ 	%error unsupported object format!
+ %endif
diff -ru flac-1.3.0/src/plugin_xmms/Makefile.in flac-1.3.0.new/src/plugin_xmms/Makefile.in
--- flac-1.3.0/src/plugin_xmms/Makefile.in	2013-05-27 10:11:57.000000000 +0200
+++ flac-1.3.0.new/src/plugin_xmms/Makefile.in	2013-10-16 13:30:02.000000000 +0200
@@ -361,7 +361,6 @@
 	$(top_builddir)/src/share/replaygain_synthesis/libreplaygain_synthesis.la \
 	$(top_builddir)/src/share/utf8/libutf8.la \
 	$(top_builddir)/src/libFLAC/libFLAC.la \
-	-L$(top_builddir)/src/libFLAC/.libs \
 	@XMMS_LIBS@ \
 	@LIBICONV@
EOF

test -f flac_1.2.1_size_t_max_patch.diff ||
  cat >flac_1.2.1_size_t_max_patch.diff <<"EOF"
diff -ru flac-1.2.1.orig/include/share/alloc.h flac-1.2.1/include/share/alloc.h
--- flac-1.2.1.orig/include/share/alloc.h	Wed Sep 12 06:32:22 2007
+++ flac-1.2.1/include/share/alloc.h	Mon Mar  3 18:57:14 2008
@@ -33,6 +33,10 @@
 #endif
 #include <stdlib.h> /* for size_t, malloc(), etc */
 
+#if defined __MINGW32__ && !defined SIZE_T_MAX
+# define SIZE_T_MAX UINT_MAX
+#endif
+
 #ifndef SIZE_MAX
 # ifndef SIZE_T_MAX
 #  ifdef _MSC_VER
EOF

test -f id3lib-3.8.3_mingw.patch ||
  cat >id3lib-3.8.3_mingw.patch <<"EOF"
diff -ru id3lib-3.8.3.orig/configure.in id3lib-3.8.3/configure.in
--- id3lib-3.8.3.orig/configure.in	2012-02-05 13:09:59 +0100
+++ id3lib-3.8.3/configure.in	2012-02-05 13:16:33 +0100
@@ -222,7 +222,7 @@
 AC_LANG_CPLUSPLUS
 AC_CHECK_HEADERS(libcw/sys.h)
 AC_CHECK_HEADERS(cctype climits cstdio cstdlib bitset cstring)
-AC_CHECK_HEADERS(fstream iostream iomanip vector \
+AC_CHECK_HEADERS(fstream iostream vector \
 	,,AC_MSG_ERROR([Missing a vital header file for id3lib - download them here: http://gcc.gnu.org/libstdc++/ or better - compile a newer compiler like gcc3.x])
 )
 AC_CHECK_HEADERS(               \
@@ -248,10 +248,10 @@
 AM_CONDITIONAL(ID3_NEEDGETOPT_LONG, test x$ac_cv_func_getopt_long = xno)
 
 AC_CHECK_FUNCS(mkstemp)
-AC_CHECK_FUNCS(
-  truncate                      \
-  ,,AC_MSG_ERROR([Missing a vital function for id3lib])
-)
+#AC_CHECK_FUNCS(
+#  truncate                      \
+#  ,,AC_MSG_ERROR([Missing a vital function for id3lib])
+#)
 
 dnl Checks for typedefs, structures, and compiler characteristics.
 AC_TYPE_SIZE_T
diff -ru id3lib-3.8.3.orig/include/id3/globals.h id3lib-3.8.3/include/id3/globals.h
--- id3lib-3.8.3.orig/include/id3/globals.h	2012-02-05 13:09:59 +0100
+++ id3lib-3.8.3/include/id3/globals.h	2012-02-05 13:15:42 +0100
@@ -41,7 +41,7 @@
  * we prefix variable declarations so they can
  * properly get exported in windows dlls.
  */
-#ifdef WIN32
+#ifdef __MSVC_VER
 #  define LINKOPTION_STATIC         1 //both for use and creation of static lib
 #  define LINKOPTION_CREATE_DYNAMIC 2 //should only be used by prj/id3lib.dsp
 #  define LINKOPTION_USE_DYNAMIC    3 //if your project links id3lib dynamic
EOF

test -f id3lib-3.8.3_wintempfile.patch ||
  cat >id3lib-3.8.3_wintempfile.patch <<"EOF"
diff -ru id3lib-3.8.3.orig/src/tag_file.cpp id3lib-3.8.3/src/tag_file.cpp
--- id3lib-3.8.3.orig/src/tag_file.cpp	2016-06-09 20:54:44.395068889 +0200
+++ id3lib-3.8.3/src/tag_file.cpp	2016-06-09 21:39:35.044411098 +0200
@@ -242,7 +242,7 @@
     strcpy(sTempFile, filename.c_str());
     strcat(sTempFile, sTmpSuffix.c_str());
 
-#if !defined(HAVE_MKSTEMP)
+#if !defined(HAVE_MKSTEMP) || defined WIN32
     // This section is for Windows folk
     fstream tmpOut;
     createFile(sTempFile, tmpOut);
EOF

test -f taglib-msvc.patch ||
cat >taglib-msvc.patch <<"EOF"
diff -ru taglib-1.8.orig/CMakeLists.txt taglib-1.8/CMakeLists.txt
--- taglib-1.8.orig/CMakeLists.txt	Thu Sep  6 20:03:15 2012
+++ taglib-1.8/CMakeLists.txt	Fri Feb 22 06:41:36 2013
@@ -31,6 +31,10 @@
 set(LIB_INSTALL_DIR "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed (default is /lib${LIB_SUFFIX})" FORCE)
 set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "The subdirectory to the header prefix" FORCE)
 
+if(MSVC)
+  add_definitions(/Zc:wchar_t-)
+endif(MSVC)
+
 if(APPLE)
 	option(BUILD_FRAMEWORK "Build an OS X framework" OFF)
 	set(FRAMEWORK_INSTALL_DIR "/Library/Frameworks" CACHE STRING "Directory to install frameworks to.")
EOF

test -f taglib_bvreplace.patch ||
  cat >taglib_bvreplace.patch <<"EOF"
--- taglib-1.9.1/taglib/toolkit/tbytevector.cpp.orig	2013-10-08 17:50:01.000000000 +0200
+++ taglib-1.9.1/taglib/toolkit/tbytevector.cpp	2015-03-17 13:03:45.267093248 +0100
@@ -31,6 +31,7 @@
 #include <iostream>
 #include <cstdio>
 #include <cstring>
+#include <cstddef>
 
 #include <tstring.h>
 #include <tdebug.h>
@@ -508,62 +509,40 @@
   if(pattern.size() == 0 || pattern.size() > size())
     return *this;
 
-  const uint withSize = with.size();
-  const uint patternSize = pattern.size();
-  int offset = 0;
+  const size_t withSize    = with.size();
+  const size_t patternSize = pattern.size();
+  const ptrdiff_t diff = withSize - patternSize;
+
+  size_t offset = 0;
+  while (true)
+  {
+    offset = find(pattern, offset);
+    if(offset == static_cast<size_t>(-1)) // Use npos in taglib2.
+      break;
 
-  if(withSize == patternSize) {
-    // I think this case might be common enough to optimize it
     detach();
-    offset = find(pattern);
-    while(offset >= 0) {
-      ::memcpy(data() + offset, with.data(), withSize);
-      offset = find(pattern, offset + withSize);
-    }
-    return *this;
-  }
 
-  // calculate new size:
-  uint newSize = 0;
-  for(;;) {
-    int next = find(pattern, offset);
-    if(next < 0) {
-      if(offset == 0)
-        // pattern not found, do nothing:
-        return *this;
-      newSize += size() - offset;
-      break;
+    if(diff < 0) {
+      ::memmove(
+        data() + offset + withSize,
+        data() + offset + patternSize,
+        size() - offset - patternSize);
+      resize(size() + diff);
     }
-    newSize += (next - offset) + withSize;
-    offset = next + patternSize;
-  }
-
-  // new private data of appropriate size:
-  ByteVectorPrivate *newData = new ByteVectorPrivate(newSize, 0);
-  char *target = DATA(newData);
-  const char *source = data();
-
-  // copy modified data into new private data:
-  offset = 0;
-  for(;;) {
-    int next = find(pattern, offset);
-    if(next < 0) {
-      ::memcpy(target, source + offset, size() - offset);
-      break;
+    else if(diff > 0) {
+      resize(size() + diff);
+      ::memmove(
+        data() + offset + withSize,
+        data() + offset + patternSize,
+        size() - diff - offset - patternSize);
     }
-    int chunkSize = next - offset;
-    ::memcpy(target, source + offset, chunkSize);
-    target += chunkSize;
-    ::memcpy(target, with.data(), withSize);
-    target += withSize;
-    offset += chunkSize + patternSize;
-  }
 
-  // replace private data:
-  if(d->deref())
-    delete d;
+    ::memcpy(data() + offset, with.data(), with.size());
 
-  d = newData;
+    offset += withSize;
+    if(offset > size() - patternSize)
+      break;
+  }
 
   return *this;
 }
EOF

test -f taglib_mp4shwm.patch ||
  cat >taglib_mp4shwm.patch <<"EOF"
diff --git a/taglib/mp4/mp4tag.cpp b/taglib/mp4/mp4tag.cpp
index a8e2e7d..0c2e5cb 100644
--- a/taglib/mp4/mp4tag.cpp
+++ b/taglib/mp4/mp4tag.cpp
@@ -71,10 +71,10 @@ MP4::Tag::Tag(TagLib::File *file, MP4::Atoms *atoms) :
       parseIntPair(atom);
     }
     else if(atom->name == "cpil" || atom->name == "pgap" || atom->name == "pcst" ||
-            atom->name == "hdvd") {
+            atom->name == "hdvd" || atom->name == "shwm") {
       parseBool(atom);
     }
-    else if(atom->name == "tmpo") {
+    else if(atom->name == "tmpo" || atom->name == "\251mvi" || atom->name == "\251mvc") {
       parseInt(atom);
     }
     else if(atom->name == "tvsn" || atom->name == "tves" || atom->name == "cnID" ||
@@ -472,10 +472,11 @@ MP4::Tag::save()
     else if(name == "disk") {
       data.append(renderIntPairNoTrailing(name.data(String::Latin1), it->second));
     }
-    else if(name == "cpil" || name == "pgap" || name == "pcst" || name == "hdvd") {
+    else if(name == "cpil" || name == "pgap" || name == "pcst" || name == "hdvd" ||
+            name == "shwm") {
       data.append(renderBool(name.data(String::Latin1), it->second));
     }
-    else if(name == "tmpo") {
+    else if(name == "tmpo" || name == "\251mvi" || name == "\251mvc") {
       data.append(renderInt(name.data(String::Latin1), it->second));
     }
     else if(name == "tvsn" || name == "tves" || name == "cnID" ||
@@ -844,6 +845,11 @@ namespace
     { "sonm", "TITLESORT" },
     { "soco", "COMPOSERSORT" },
     { "sosn", "SHOWSORT" },
+    { "shwm", "SHOWWORKMOVEMENT" },
+    { "\251wrk", "WORK" },
+    { "\251mvn", "MOVEMENTNAME" },
+    { "\251mvi", "MOVEMENTNUMBER" },
+    { "\251mvc", "MOVEMENTCOUNT" },
     { "----:com.apple.iTunes:MusicBrainz Track Id", "MUSICBRAINZ_TRACKID" },
     { "----:com.apple.iTunes:MusicBrainz Artist Id", "MUSICBRAINZ_ARTISTID" },
     { "----:com.apple.iTunes:MusicBrainz Album Id", "MUSICBRAINZ_ALBUMID" },
@@ -897,10 +903,10 @@ PropertyMap MP4::Tag::properties() const
         }
         props[key] = value;
       }
-      else if(key == "BPM") {
+      else if(key == "BPM" || key == "MOVEMENTNUMBER" || key == "MOVEMENTCOUNT") {
         props[key] = String::number(it->second.toInt());
       }
-      else if(key == "COMPILATION") {
+      else if(key == "COMPILATION" || key == "SHOWWORKMOVEMENT") {
         props[key] = String::number(it->second.toBool());
       }
       else {
@@ -952,11 +958,11 @@ PropertyMap MP4::Tag::setProperties(const PropertyMap &props)
           d->items[name] = MP4::Item(first, second);
         }
       }
-      else if(it->first == "BPM" && !it->second.isEmpty()) {
+      else if((it->first == "BPM" || it->first == "MOVEMENTNUMBER" || it->first == "MOVEMENTCOUNT") && !it->second.isEmpty()) {
         int value = it->second.front().toInt();
         d->items[name] = MP4::Item(value);
       }
-      else if(it->first == "COMPILATION" && !it->second.isEmpty()) {
+      else if((it->first == "COMPILATION" || it->first == "SHOWWORKMOVEMENT") && !it->second.isEmpty()) {
         bool value = (it->second.front().toInt() != 0);
         d->items[name] = MP4::Item(value);
       }
diff --git a/taglib/mp4/mp4tag.h b/taglib/mp4/mp4tag.h
index d477a86..bca3021 100644
--- a/taglib/mp4/mp4tag.h
+++ b/taglib/mp4/mp4tag.h
@@ -35,6 +35,8 @@
 #include "mp4atom.h"
 #include "mp4item.h"
 
+#define TAGLIB_WITH_MP4_SHWM 1
+
 namespace TagLib {
 
   namespace MP4 {
diff --git a/taglib/mpeg/id3v2/id3v2frame.cpp b/taglib/mpeg/id3v2/id3v2frame.cpp
index 1f896fa..ec3b931 100644
--- a/taglib/mpeg/id3v2/id3v2frame.cpp
+++ b/taglib/mpeg/id3v2/id3v2frame.cpp
@@ -111,8 +111,8 @@ Frame *Frame::createTextualFrame(const String &key, const StringList &values) //
   // check if the key is contained in the key<=>frameID mapping
   ByteVector frameID = keyToFrameID(key);
   if(!frameID.isEmpty()) {
-    // Apple proprietary WFED (Podcast URL) is in fact a text frame.
-    if(frameID[0] == 'T' || frameID == "WFED"){ // text frame
+    // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number) are in fact text frames.
+    if(frameID[0] == 'T' || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN"){ // text frame
       TextIdentificationFrame *frame = new TextIdentificationFrame(frameID, String::UTF8);
       frame->setText(values);
       return frame;
@@ -392,6 +392,8 @@ namespace
     { "TDES", "PODCASTDESC" },
     { "TGID", "PODCASTID" },
     { "WFED", "PODCASTURL" },
+    { "MVNM", "MOVEMENTNAME" },
+    { "MVIN", "MOVEMENTNUMBER" },
   };
   const size_t frameTranslationSize = sizeof(frameTranslation) / sizeof(frameTranslation[0]);
 
@@ -474,8 +476,8 @@ PropertyMap Frame::asProperties() const
   // workaround until this function is virtual
   if(id == "TXXX")
     return dynamic_cast< const UserTextIdentificationFrame* >(this)->asProperties();
-  // Apple proprietary WFED (Podcast URL) is in fact a text frame.
-  else if(id[0] == 'T' || id == "WFED")
+  // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number) are in fact text frames.
+  else if(id[0] == 'T' || id == "WFED" || id == "MVNM" || id == "MVIN")
     return dynamic_cast< const TextIdentificationFrame* >(this)->asProperties();
   else if(id == "WXXX")
     return dynamic_cast< const UserUrlLinkFrame* >(this)->asProperties();
diff --git a/taglib/mpeg/id3v2/id3v2framefactory.cpp b/taglib/mpeg/id3v2/id3v2framefactory.cpp
index 0fbb87d..759a9b7 100644
--- a/taglib/mpeg/id3v2/id3v2framefactory.cpp
+++ b/taglib/mpeg/id3v2/id3v2framefactory.cpp
@@ -198,8 +198,8 @@ Frame *FrameFactory::createFrame(const ByteVector &origData, Header *tagHeader)
 
   // Text Identification (frames 4.2)
 
-  // Apple proprietary WFED (Podcast URL) is in fact a text frame.
-  if(frameID.startsWith("T") || frameID == "WFED") {
+  // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number) are in fact text frames.
+  if(frameID.startsWith("T") || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN") {
 
     TextIdentificationFrame *f = frameID != "TXXX"
       ? new TextIdentificationFrame(data, header)
@@ -456,6 +456,8 @@ namespace
     { "TDS", "TDES" },
     { "TID", "TGID" },
     { "WFD", "WFED" },
+    { "MVN", "MVNM" },
+    { "MVI", "MVIN" },
   };
   const size_t frameConversion2Size = sizeof(frameConversion2) / sizeof(frameConversion2[0]);
 
EOF

test -f taglib_CVE-2017-12678.patch ||
  cat >taglib_CVE-2017-12678.patch <<"EOF"
Index: b/taglib/mpeg/id3v2/id3v2framefactory.cpp
===================================================================
--- a/taglib/mpeg/id3v2/id3v2framefactory.cpp
+++ b/taglib/mpeg/id3v2/id3v2framefactory.cpp
@@ -334,10 +334,11 @@ void FrameFactory::rebuildAggregateFrame
      tag->frameList("TDAT").size() == 1)
   {
     TextIdentificationFrame *tdrc =
-      static_cast<TextIdentificationFrame *>(tag->frameList("TDRC").front());
+      dynamic_cast<TextIdentificationFrame *>(tag->frameList("TDRC").front());
     UnknownFrame *tdat = static_cast<UnknownFrame *>(tag->frameList("TDAT").front());
 
-    if(tdrc->fieldList().size() == 1 &&
+    if(tdrc &&
+       tdrc->fieldList().size() == 1 &&
        tdrc->fieldList().front().size() == 4 &&
        tdat->data().size() >= 5)
     {
EOF

test -f taglib_cmID_purl_egid.patch.patch ||
  cat >taglib_cmID_purl_egid.patch.patch <<"EOF"
index 11d3cc51..a3636a9d 100644
--- a/taglib/mp4/mp4tag.cpp
+++ b/taglib/mp4/mp4tag.cpp
@@ -78,7 +78,8 @@ MP4::Tag::Tag(TagLib::File *file, MP4::Atoms *atoms) :
       parseInt(atom);
     }
     else if(atom->name == "tvsn" || atom->name == "tves" || atom->name == "cnID" ||
-            atom->name == "sfID" || atom->name == "atID" || atom->name == "geID") {
+            atom->name == "sfID" || atom->name == "atID" || atom->name == "geID" ||
+            atom->name == "cmID") {
       parseUInt(atom);
     }
     else if(atom->name == "plID") {
@@ -93,6 +94,9 @@ MP4::Tag::Tag(TagLib::File *file, MP4::Atoms *atoms) :
     else if(atom->name == "covr") {
       parseCovr(atom);
     }
+    else if(atom->name == "purl" || atom->name == "egid") {
+      parseText(atom, -1);
+    }
     else {
       parseText(atom);
     }
@@ -480,7 +484,8 @@ MP4::Tag::save()
       data.append(renderInt(name.data(String::Latin1), it->second));
     }
     else if(name == "tvsn" || name == "tves" || name == "cnID" ||
-            name == "sfID" || name == "atID" || name == "geID") {
+            name == "sfID" || name == "atID" || name == "geID" ||
+            name == "cmID") {
       data.append(renderUInt(name.data(String::Latin1), it->second));
     }
     else if(name == "plID") {
@@ -492,6 +497,9 @@ MP4::Tag::save()
     else if(name == "covr") {
       data.append(renderCovr(name.data(String::Latin1), it->second));
     }
+    else if(name == "purl" || name == "egid") {
+      data.append(renderText(name.data(String::Latin1), it->second, TypeImplicit));
+    }
     else if(name.size() == 4){
       data.append(renderText(name.data(String::Latin1), it->second));
     }
EOF

test -f taglib_grp1.patch ||
  cat >taglib_grp1.patch <<"EOF"
diff --git a/taglib/mpeg/id3v2/id3v2frame.cpp b/taglib/mpeg/id3v2/id3v2frame.cpp
index 4f88dec1..af4136af 100644
--- a/taglib/mpeg/id3v2/id3v2frame.cpp
+++ b/taglib/mpeg/id3v2/id3v2frame.cpp
@@ -111,8 +111,8 @@ Frame *Frame::createTextualFrame(const String &key, const StringList &values) //
   // check if the key is contained in the key<=>frameID mapping
   ByteVector frameID = keyToFrameID(key);
   if(!frameID.isEmpty()) {
-    // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number) are in fact text frames.
-    if(frameID[0] == 'T' || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN"){ // text frame
+    // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number), GRP1 (Grouping) are in fact text frames.
+    if(frameID[0] == 'T' || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN" || frameID == "GRP1"){ // text frame
       TextIdentificationFrame *frame = new TextIdentificationFrame(frameID, String::UTF8);
       frame->setText(values);
       return frame;
@@ -394,6 +394,7 @@ namespace
     { "WFED", "PODCASTURL" },
     { "MVNM", "MOVEMENTNAME" },
     { "MVIN", "MOVEMENTNUMBER" },
+    { "GRP1", "GROUPING" },
   };
   const size_t frameTranslationSize = sizeof(frameTranslation) / sizeof(frameTranslation[0]);
 
@@ -476,8 +477,8 @@ PropertyMap Frame::asProperties() const
   // workaround until this function is virtual
   if(id == "TXXX")
     return dynamic_cast< const UserTextIdentificationFrame* >(this)->asProperties();
-  // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number) are in fact text frames.
-  else if(id[0] == 'T' || id == "WFED" || id == "MVNM" || id == "MVIN")
+  // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number), GRP1 (Grouping) are in fact text frames.
+  else if(id[0] == 'T' || id == "WFED" || id == "MVNM" || id == "MVIN" || id == "GRP1")
     return dynamic_cast< const TextIdentificationFrame* >(this)->asProperties();
   else if(id == "WXXX")
     return dynamic_cast< const UserUrlLinkFrame* >(this)->asProperties();
diff --git a/taglib/mpeg/id3v2/id3v2framefactory.cpp b/taglib/mpeg/id3v2/id3v2framefactory.cpp
index 9347ab86..155d0a9d 100644
--- a/taglib/mpeg/id3v2/id3v2framefactory.cpp
+++ b/taglib/mpeg/id3v2/id3v2framefactory.cpp
@@ -198,8 +198,8 @@ Frame *FrameFactory::createFrame(const ByteVector &origData, Header *tagHeader)
 
   // Text Identification (frames 4.2)
 
-  // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number) are in fact text frames.
-  if(frameID.startsWith("T") || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN") {
+  // Apple proprietary WFED (Podcast URL), MVNM (Movement Name), MVIN (Movement Number), GRP1 (Grouping) are in fact text frames.
+  if(frameID.startsWith("T") || frameID == "WFED" || frameID == "MVNM" || frameID == "MVIN" || frameID == "GRP1") {
 
     TextIdentificationFrame *f = frameID != "TXXX"
       ? new TextIdentificationFrame(data, header)
@@ -459,6 +459,7 @@ namespace
     { "WFD", "WFED" },
     { "MVN", "MVNM" },
     { "MVI", "MVIN" },
+    { "GP1", "GRP1" },
   };
   const size_t frameConversion2Size = sizeof(frameConversion2) / sizeof(frameConversion2[0]);
EOF

test -f taglib_oggbitrate.patch ||
  cat >taglib_oggbitrate.patch <<"EOF"
diff --git a/taglib/ogg/opus/opusproperties.cpp b/taglib/ogg/opus/opusproperties.cpp
index 537ba166..b60cc01d 100644
--- a/taglib/ogg/opus/opusproperties.cpp
+++ b/taglib/ogg/opus/opusproperties.cpp
@@ -163,8 +163,14 @@ void Opus::Properties::read(File *file)
 
       if(frameCount > 0) {
         const double length = frameCount * 1000.0 / 48000.0;
+        long fileLengthWithoutOverhead = file->length();
+        // Ignore the two mandatory header packets, see "3. Packet Organization"
+        // in https://tools.ietf.org/html/rfc7845.html
+        for (unsigned int i = 0; i < 2; ++i) {
+          fileLengthWithoutOverhead -= file->packet(i).size();
+        }
         d->length  = static_cast<int>(length + 0.5);
-        d->bitrate = static_cast<int>(file->length() * 8.0 / length + 0.5);
+        d->bitrate = static_cast<int>(fileLengthWithoutOverhead * 8.0 / length + 0.5);
       }
     }
     else {
diff --git a/taglib/ogg/speex/speexproperties.cpp b/taglib/ogg/speex/speexproperties.cpp
index fbcc5a4b..b7a11cc6 100644
--- a/taglib/ogg/speex/speexproperties.cpp
+++ b/taglib/ogg/speex/speexproperties.cpp
@@ -182,8 +182,14 @@ void Speex::Properties::read(File *file)
 
       if(frameCount > 0) {
         const double length = frameCount * 1000.0 / d->sampleRate;
+        long fileLengthWithoutOverhead = file->length();
+        // Ignore the two header packets, see "Ogg file format" in
+        // https://www.speex.org/docs/manual/speex-manual/node8.html
+        for (unsigned int i = 0; i < 2; ++i) {
+          fileLengthWithoutOverhead -= file->packet(i).size();
+        }
         d->length  = static_cast<int>(length + 0.5);
-        d->bitrate = static_cast<int>(file->length() * 8.0 / length + 0.5);
+        d->bitrate = static_cast<int>(fileLengthWithoutOverhead * 8.0 / length + 0.5);
       }
     }
     else {
diff --git a/taglib/ogg/vorbis/vorbisproperties.cpp b/taglib/ogg/vorbis/vorbisproperties.cpp
index 981400f0..4000c254 100644
--- a/taglib/ogg/vorbis/vorbisproperties.cpp
+++ b/taglib/ogg/vorbis/vorbisproperties.cpp
@@ -186,9 +186,14 @@ void Vorbis::Properties::read(File *file)
 
       if(frameCount > 0) {
         const double length = frameCount * 1000.0 / d->sampleRate;
-
+        long fileLengthWithoutOverhead = file->length();
+        // Ignore the three initial header packets, see "1.3.1. Decode Setup" in
+        // https://xiph.org/vorbis/doc/Vorbis_I_spec.html
+        for (unsigned int i = 0; i < 3; ++i) {
+          fileLengthWithoutOverhead -= file->packet(i).size();
+        }
         d->length  = static_cast<int>(length + 0.5);
-        d->bitrate = static_cast<int>(file->length() * 8.0 / length + 0.5);
+        d->bitrate = static_cast<int>(fileLengthWithoutOverhead * 8.0 / length + 0.5);
       }
     }
     else {
diff --git a/tests/test_ogg.cpp b/tests/test_ogg.cpp
index 5569e59c..ebb865fd 100644
--- a/tests/test_ogg.cpp
+++ b/tests/test_ogg.cpp
@@ -191,7 +191,7 @@ public:
     CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->length());
     CPPUNIT_ASSERT_EQUAL(3, f.audioProperties()->lengthInSeconds());
     CPPUNIT_ASSERT_EQUAL(3685, f.audioProperties()->lengthInMilliseconds());
-    CPPUNIT_ASSERT_EQUAL(9, f.audioProperties()->bitrate());
+    CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->bitrate());
     CPPUNIT_ASSERT_EQUAL(2, f.audioProperties()->channels());
     CPPUNIT_ASSERT_EQUAL(44100, f.audioProperties()->sampleRate());
     CPPUNIT_ASSERT_EQUAL(0, f.audioProperties()->vorbisVersion());
diff --git a/tests/test_opus.cpp b/tests/test_opus.cpp
index 9d14df23..cdf77eae 100644
--- a/tests/test_opus.cpp
+++ b/tests/test_opus.cpp
@@ -53,7 +53,7 @@ public:
     CPPUNIT_ASSERT_EQUAL(7, f.audioProperties()->length());
     CPPUNIT_ASSERT_EQUAL(7, f.audioProperties()->lengthInSeconds());
     CPPUNIT_ASSERT_EQUAL(7737, f.audioProperties()->lengthInMilliseconds());
-    CPPUNIT_ASSERT_EQUAL(37, f.audioProperties()->bitrate());
+    CPPUNIT_ASSERT_EQUAL(36, f.audioProperties()->bitrate());
     CPPUNIT_ASSERT_EQUAL(1, f.audioProperties()->channels());
     CPPUNIT_ASSERT_EQUAL(48000, f.audioProperties()->sampleRate());
     CPPUNIT_ASSERT_EQUAL(48000, f.audioProperties()->inputSampleRate());
EOF

test -f taglib_large_file.patch ||
  cat >taglib_large_file.patch <<"EOF"
From 422d3ef95d4aff4975c334800aa9da4cb0c783a8 Mon Sep 17 00:00:00 2001
From: Urs Fleisch <ufleisch@users.sourceforge.net>
Date: Sun, 19 Apr 2020 11:13:55 +0200
Subject: Support large files over 2GB

Backport of 4dcf0b41c687292e7b1263a679921c157ae2c22a
b01f45e141afa6a89aea319a2783f177e202fa1d
https://github.com/taglib/taglib/pull/77

diff --git a/taglib/ape/apefile.cpp b/taglib/ape/apefile.cpp
index 9f298aaf..8c1592eb 100644
--- a/taglib/ape/apefile.cpp
+++ b/taglib/ape/apefile.cpp
@@ -69,13 +69,13 @@ public:
     delete properties;
   }
 
-  long APELocation;
+  offset_t APELocation;
   long APESize;
 
-  long ID3v1Location;
+  offset_t ID3v1Location;
 
   ID3v2::Header *ID3v2Header;
-  long ID3v2Location;
+  offset_t ID3v2Location;
   long ID3v2Size;
 
   TagUnion tag;
@@ -280,7 +280,7 @@ void APE::File::read(bool readProperties)
 
   if(readProperties) {
 
-    long streamLength;
+    offset_t streamLength;
 
     if(d->APELocation >= 0)
       streamLength = d->APELocation;
diff --git a/taglib/ape/apeproperties.cpp b/taglib/ape/apeproperties.cpp
index dee7e8c0..5a43445e 100644
--- a/taglib/ape/apeproperties.cpp
+++ b/taglib/ape/apeproperties.cpp
@@ -70,7 +70,7 @@ APE::Properties::Properties(File *, ReadStyle style) :
   debug("APE::Properties::Properties() -- This constructor is no longer used.");
 }
 
-APE::Properties::Properties(File *file, long streamLength, ReadStyle style) :
+APE::Properties::Properties(File *file, offset_t streamLength, ReadStyle style) :
   AudioProperties(style),
   d(new PropertiesPrivate())
 {
@@ -142,10 +142,10 @@ namespace
   }
 }
 
-void APE::Properties::read(File *file, long streamLength)
+void APE::Properties::read(File *file, offset_t streamLength)
 {
   // First, we assume that the file pointer is set at the first descriptor.
-  long offset = file->tell();
+  offset_t offset = file->tell();
   int version = headerVersion(file->readBlock(6));
 
   // Next, we look for the descriptor.
diff --git a/taglib/ape/apeproperties.h b/taglib/ape/apeproperties.h
index 91625483..0e4856db 100644
--- a/taglib/ape/apeproperties.h
+++ b/taglib/ape/apeproperties.h
@@ -61,7 +61,7 @@ namespace TagLib {
        * Create an instance of APE::Properties with the data read from the
        * APE::File \a file.
        */
-      Properties(File *file, long streamLength, ReadStyle style = Average);
+      Properties(File *file, offset_t streamLength, ReadStyle style = Average);
 
       /*!
        * Destroys this APE::Properties instance.
@@ -129,7 +129,7 @@ namespace TagLib {
       Properties(const Properties &);
       Properties &operator=(const Properties &);
 
-      void read(File *file, long streamLength);
+      void read(File *file, offset_t streamLength);
 
       void analyzeCurrent(File *file);
       void analyzeOld(File *file);
diff --git a/taglib/ape/apetag.cpp b/taglib/ape/apetag.cpp
index 89ef8ff4..805274b1 100644
--- a/taglib/ape/apetag.cpp
+++ b/taglib/ape/apetag.cpp
@@ -79,7 +79,7 @@ public:
     footerLocation(0) {}
 
   File *file;
-  long footerLocation;
+  offset_t footerLocation;
 
   Footer footer;
   ItemListMap itemListMap;
@@ -95,7 +95,7 @@ APE::Tag::Tag() :
 {
 }
 
-APE::Tag::Tag(TagLib::File *file, long footerLocation) :
+APE::Tag::Tag(TagLib::File *file, offset_t footerLocation) :
   TagLib::Tag(),
   d(new TagPrivate())
 {
diff --git a/taglib/ape/apetag.h b/taglib/ape/apetag.h
index f4d4fba6..b5a2eb87 100644
--- a/taglib/ape/apetag.h
+++ b/taglib/ape/apetag.h
@@ -66,7 +66,7 @@ namespace TagLib {
        * Create an APE tag and parse the data in \a file with APE footer at
        * \a tagOffset.
        */
-      Tag(TagLib::File *file, long footerLocation);
+      Tag(TagLib::File *file, offset_t footerLocation);
 
       /*!
        * Destroys this Tag instance.
diff --git a/taglib/flac/flacfile.cpp b/taglib/flac/flacfile.cpp
index b31cc65e..29e2f613 100644
--- a/taglib/flac/flacfile.cpp
+++ b/taglib/flac/flacfile.cpp
@@ -79,10 +79,10 @@ public:
   }
 
   const ID3v2::FrameFactory *ID3v2FrameFactory;
-  long ID3v2Location;
+  offset_t ID3v2Location;
   long ID3v2OriginalSize;
 
-  long ID3v1Location;
+  offset_t ID3v1Location;
 
   TagUnion tag;
 
@@ -90,8 +90,8 @@ public:
   ByteVector xiphCommentData;
   BlockList blocks;
 
-  long flacStart;
-  long streamStart;
+  offset_t flacStart;
+  offset_t streamStart;
   bool scanned;
 };
 
@@ -326,7 +326,7 @@ ByteVector FLAC::File::streamInfoData()
   return ByteVector();
 }
 
-long FLAC::File::streamLength()
+offset_t FLAC::File::streamLength()
 {
   debug("FLAC::File::streamLength() -- This function is obsolete. Returning zero.");
   return 0;
@@ -441,7 +441,7 @@ void FLAC::File::read(bool readProperties)
 
     const ByteVector infoData = d->blocks.front()->render();
 
-    long streamLength;
+    offset_t streamLength;
 
     if(d->ID3v1Location >= 0)
       streamLength = d->ID3v1Location - d->streamStart;
@@ -462,7 +462,7 @@ void FLAC::File::scan()
   if(!isValid())
     return;
 
-  long nextBlockOffset;
+  offset_t nextBlockOffset;
 
   if(d->ID3v2Location >= 0)
     nextBlockOffset = find("fLaC", d->ID3v2Location + d->ID3v2OriginalSize);
diff --git a/taglib/flac/flacfile.h b/taglib/flac/flacfile.h
index 65d85679..c1464c45 100644
--- a/taglib/flac/flacfile.h
+++ b/taglib/flac/flacfile.h
@@ -256,7 +256,7 @@ namespace TagLib {
        *
        * \deprecated Always returns zero.
        */
-      long streamLength();  // BIC: remove
+      offset_t streamLength();  // BIC: remove
 
       /*!
        * Returns a list of pictures attached to the FLAC file.
diff --git a/taglib/flac/flacproperties.cpp b/taglib/flac/flacproperties.cpp
index b947f039..a798940a 100644
--- a/taglib/flac/flacproperties.cpp
+++ b/taglib/flac/flacproperties.cpp
@@ -55,7 +55,7 @@ public:
 // public members
 ////////////////////////////////////////////////////////////////////////////////
 
-FLAC::Properties::Properties(ByteVector data, long streamLength, ReadStyle style) :
+FLAC::Properties::Properties(ByteVector data, offset_t streamLength, ReadStyle style) :
   AudioProperties(style),
   d(new PropertiesPrivate())
 {
@@ -128,7 +128,7 @@ ByteVector FLAC::Properties::signature() const
 // private members
 ////////////////////////////////////////////////////////////////////////////////
 
-void FLAC::Properties::read(const ByteVector &data, long streamLength)
+void FLAC::Properties::read(const ByteVector &data, offset_t streamLength)
 {
   if(data.size() < 18) {
     debug("FLAC::Properties::read() - FLAC properties must contain at least 18 bytes.");
diff --git a/taglib/flac/flacproperties.h b/taglib/flac/flacproperties.h
index 6f13ce62..1dd0fc5f 100644
--- a/taglib/flac/flacproperties.h
+++ b/taglib/flac/flacproperties.h
@@ -50,7 +50,7 @@ namespace TagLib {
        * ByteVector \a data.
        */
        // BIC: switch to const reference
-      Properties(ByteVector data, long streamLength, ReadStyle style = Average);
+      Properties(ByteVector data, offset_t streamLength, ReadStyle style = Average);
 
       /*!
        * Create an instance of FLAC::Properties with the data read from the
@@ -137,7 +137,7 @@ namespace TagLib {
       Properties(const Properties &);
       Properties &operator=(const Properties &);
 
-      void read(const ByteVector &data, long streamLength);
+      void read(const ByteVector &data, offset_t streamLength);
 
       class PropertiesPrivate;
       PropertiesPrivate *d;
diff --git a/taglib/mp4/mp4atom.cpp b/taglib/mp4/mp4atom.cpp
index 6ea0cb62..18dbbed9 100644
--- a/taglib/mp4/mp4atom.cpp
+++ b/taglib/mp4/mp4atom.cpp
@@ -155,7 +155,7 @@ MP4::Atoms::Atoms(File *file)
   atoms.setAutoDelete(true);
 
   file->seek(0, File::End);
-  long end = file->tell();
+  offset_t end = file->tell();
   file->seek(0);
   while(file->tell() + 8 <= end) {
     MP4::Atom *atom = new MP4::Atom(file);
diff --git a/taglib/mp4/mp4atom.h b/taglib/mp4/mp4atom.h
index cbb0d10a..53bc16a6 100644
--- a/taglib/mp4/mp4atom.h
+++ b/taglib/mp4/mp4atom.h
@@ -82,8 +82,8 @@ namespace TagLib {
       Atom *find(const char *name1, const char *name2 = 0, const char *name3 = 0, const char *name4 = 0);
       bool path(AtomList &path, const char *name1, const char *name2 = 0, const char *name3 = 0);
       AtomList findall(const char *name, bool recursive = false);
-      long offset;
-      long length;
+      offset_t offset;
+      offset_t length;
       TagLib::ByteVector name;
       AtomList children;
     private:
diff --git a/taglib/mp4/mp4tag.cpp b/taglib/mp4/mp4tag.cpp
index 9cbabbd1..fb871b48 100644
--- a/taglib/mp4/mp4tag.cpp
+++ b/taglib/mp4/mp4tag.cpp
@@ -549,7 +549,7 @@ MP4::Tag::updateParents(const AtomList &path, long delta, int ignore)
 }
 
 void
-MP4::Tag::updateOffsets(long delta, long offset)
+MP4::Tag::updateOffsets(long delta, offset_t offset)
 {
   MP4::Atom *moov = d->atoms->find("moov");
   if(moov) {
@@ -633,7 +633,7 @@ MP4::Tag::saveNew(ByteVector data)
     data = renderAtom("udta", data);
   }
 
-  long offset = path.back()->offset + 8;
+  offset_t offset = path.back()->offset + 8;
   d->file->insert(data, offset, 0);
 
   updateParents(path, data.size());
@@ -651,8 +651,8 @@ MP4::Tag::saveExisting(ByteVector data, const AtomList &path)
   AtomList::ConstIterator it = path.end();
 
   MP4::Atom *ilst = *(--it);
-  long offset = ilst->offset;
-  long length = ilst->length;
+  offset_t offset = ilst->offset;
+  offset_t length = ilst->length;
 
   MP4::Atom *meta = *(--it);
   AtomList::ConstIterator index = meta->children.find(ilst);
diff --git a/taglib/mp4/mp4tag.h b/taglib/mp4/mp4tag.h
index bca30217..1b7a3d13 100644
--- a/taglib/mp4/mp4tag.h
+++ b/taglib/mp4/mp4tag.h
@@ -141,7 +141,7 @@ namespace TagLib {
         ByteVector renderCovr(const ByteVector &name, const Item &item) const;
 
         void updateParents(const AtomList &path, long delta, int ignore = 0);
-        void updateOffsets(long delta, long offset);
+        void updateOffsets(long delta, offset_t offset);
 
         void saveNew(ByteVector data);
         void saveExisting(ByteVector data, const AtomList &path);
diff --git a/taglib/mpc/mpcfile.cpp b/taglib/mpc/mpcfile.cpp
index daf24c8f..0d8a23c8 100644
--- a/taglib/mpc/mpcfile.cpp
+++ b/taglib/mpc/mpcfile.cpp
@@ -61,13 +61,13 @@ public:
     delete properties;
   }
 
-  long APELocation;
+  offset_t APELocation;
   long APESize;
 
-  long ID3v1Location;
+  offset_t ID3v1Location;
 
   ID3v2::Header *ID3v2Header;
-  long ID3v2Location;
+  offset_t ID3v2Location;
   long ID3v2Size;
 
   TagUnion tag;
@@ -297,7 +297,7 @@ void MPC::File::read(bool readProperties)
 
   if(readProperties) {
 
-    long streamLength;
+    offset_t streamLength;
 
     if(d->APELocation >= 0)
       streamLength = d->APELocation;
diff --git a/taglib/mpc/mpcproperties.cpp b/taglib/mpc/mpcproperties.cpp
index b9fbbf13..a7744b10 100644
--- a/taglib/mpc/mpcproperties.cpp
+++ b/taglib/mpc/mpcproperties.cpp
@@ -67,14 +67,14 @@ public:
 // public members
 ////////////////////////////////////////////////////////////////////////////////
 
-MPC::Properties::Properties(const ByteVector &data, long streamLength, ReadStyle style) :
+MPC::Properties::Properties(const ByteVector &data, offset_t streamLength, ReadStyle style) :
   AudioProperties(style),
   d(new PropertiesPrivate())
 {
   readSV7(data, streamLength);
 }
 
-MPC::Properties::Properties(File *file, long streamLength, ReadStyle style) :
+MPC::Properties::Properties(File *file, offset_t streamLength, ReadStyle style) :
   AudioProperties(style),
   d(new PropertiesPrivate())
 {
@@ -204,7 +204,7 @@ namespace
   const unsigned short sftable [8] = { 44100, 48000, 37800, 32000, 0, 0, 0, 0 };
 }
 
-void MPC::Properties::readSV8(File *file, long streamLength)
+void MPC::Properties::readSV8(File *file, offset_t streamLength)
 {
   bool readSH = false, readRG = false;
 
@@ -296,7 +296,7 @@ void MPC::Properties::readSV8(File *file, long streamLength)
   }
 }
 
-void MPC::Properties::readSV7(const ByteVector &data, long streamLength)
+void MPC::Properties::readSV7(const ByteVector &data, offset_t streamLength)
 {
   if(data.startsWith("MP+")) {
     d->version = data[3] & 15;
diff --git a/taglib/mpc/mpcproperties.h b/taglib/mpc/mpcproperties.h
index d5fdfbb9..4c69d66b 100644
--- a/taglib/mpc/mpcproperties.h
+++ b/taglib/mpc/mpcproperties.h
@@ -53,13 +53,13 @@ namespace TagLib {
        *
        * This constructor is deprecated. It only works for MPC version up to 7.
        */
-      Properties(const ByteVector &data, long streamLength, ReadStyle style = Average);
+      Properties(const ByteVector &data, offset_t streamLength, ReadStyle style = Average);
 
       /*!
        * Create an instance of MPC::Properties with the data read directly
        * from a MPC::File.
        */
-      Properties(File *file, long streamLength, ReadStyle style = Average);
+      Properties(File *file, offset_t streamLength, ReadStyle style = Average);
 
       /*!
        * Destroys this MPC::Properties instance.
@@ -146,8 +146,8 @@ namespace TagLib {
       Properties(const Properties &);
       Properties &operator=(const Properties &);
 
-      void readSV7(const ByteVector &data, long streamLength);
-      void readSV8(File *file, long streamLength);
+      void readSV7(const ByteVector &data, offset_t streamLength);
+      void readSV8(File *file, offset_t streamLength);
 
       class PropertiesPrivate;
       PropertiesPrivate *d;
diff --git a/taglib/mpeg/id3v1/id3v1tag.cpp b/taglib/mpeg/id3v1/id3v1tag.cpp
index ca930411..4d26b3a4 100644
--- a/taglib/mpeg/id3v1/id3v1tag.cpp
+++ b/taglib/mpeg/id3v1/id3v1tag.cpp
@@ -48,7 +48,7 @@ public:
     genre(255) {}
 
   File *file;
-  long tagOffset;
+  offset_t tagOffset;
 
   String title;
   String artist;
@@ -90,7 +90,7 @@ ID3v1::Tag::Tag() :
 {
 }
 
-ID3v1::Tag::Tag(File *file, long tagOffset) :
+ID3v1::Tag::Tag(File *file, offset_t tagOffset) :
   TagLib::Tag(),
   d(new TagPrivate())
 {
diff --git a/taglib/mpeg/id3v1/id3v1tag.h b/taglib/mpeg/id3v1/id3v1tag.h
index b61f06af..55ab63ff 100644
--- a/taglib/mpeg/id3v1/id3v1tag.h
+++ b/taglib/mpeg/id3v1/id3v1tag.h
@@ -114,7 +114,7 @@ namespace TagLib {
        * Create an ID3v1 tag and parse the data in \a file starting at
        * \a tagOffset.
        */
-      Tag(File *file, long tagOffset);
+      Tag(File *file, offset_t tagOffset);
 
       /*!
        * Destroys this Tag instance.
diff --git a/taglib/mpeg/id3v2/id3v2tag.cpp b/taglib/mpeg/id3v2/id3v2tag.cpp
index 4c00ab6f..ba27a123 100644
--- a/taglib/mpeg/id3v2/id3v2tag.cpp
+++ b/taglib/mpeg/id3v2/id3v2tag.cpp
@@ -77,7 +77,7 @@ public:
   const FrameFactory *factory;
 
   File *file;
-  long tagOffset;
+  offset_t tagOffset;
 
   Header header;
   ExtendedHeader *extendedHeader;
@@ -115,7 +115,7 @@ ID3v2::Tag::Tag() :
   d->factory = FrameFactory::instance();
 }
 
-ID3v2::Tag::Tag(File *file, long tagOffset, const FrameFactory *factory) :
+ID3v2::Tag::Tag(File *file, offset_t tagOffset, const FrameFactory *factory) :
   TagLib::Tag(),
   d(new TagPrivate())
 {
diff --git a/taglib/mpeg/id3v2/id3v2tag.h b/taglib/mpeg/id3v2/id3v2tag.h
index 4367181f..503ad659 100644
--- a/taglib/mpeg/id3v2/id3v2tag.h
+++ b/taglib/mpeg/id3v2/id3v2tag.h
@@ -154,7 +154,7 @@ namespace TagLib {
        *
        * \see FrameFactory
        */
-      Tag(File *file, long tagOffset,
+      Tag(File *file, offset_t tagOffset,
           const FrameFactory *factory = FrameFactory::instance());
 
       /*!
diff --git a/taglib/mpeg/mpegfile.cpp b/taglib/mpeg/mpegfile.cpp
index af7253fa..c512e8e6 100644
--- a/taglib/mpeg/mpegfile.cpp
+++ b/taglib/mpeg/mpegfile.cpp
@@ -63,13 +63,13 @@ public:
 
   const ID3v2::FrameFactory *ID3v2FrameFactory;
 
-  long ID3v2Location;
+  offset_t ID3v2Location;
   long ID3v2OriginalSize;
 
-  long APELocation;
+  offset_t APELocation;
   long APEOriginalSize;
 
-  long ID3v1Location;
+  offset_t ID3v1Location;
 
   TagUnion tag;
 
@@ -344,7 +344,7 @@ void MPEG::File::setID3v2FrameFactory(const ID3v2::FrameFactory *factory)
   d->ID3v2FrameFactory = factory;
 }
 
-long MPEG::File::nextFrameOffset(long position)
+offset_t MPEG::File::nextFrameOffset(offset_t position)
 {
   bool foundLastSyncPattern = false;
 
@@ -370,7 +370,7 @@ long MPEG::File::nextFrameOffset(long position)
   }
 }
 
-long MPEG::File::previousFrameOffset(long position)
+offset_t MPEG::File::previousFrameOffset(offset_t position)
 {
   bool foundFirstSyncPattern = false;
   ByteVector buffer;
@@ -398,7 +398,7 @@ long MPEG::File::previousFrameOffset(long position)
   return -1;
 }
 
-long MPEG::File::firstFrameOffset()
+offset_t MPEG::File::firstFrameOffset()
 {
   long position = 0;
 
@@ -408,9 +408,9 @@ long MPEG::File::firstFrameOffset()
   return nextFrameOffset(position);
 }
 
-long MPEG::File::lastFrameOffset()
+offset_t MPEG::File::lastFrameOffset()
 {
-  long position;
+  offset_t position;
 
   if(hasAPETag())
     position = d->APELocation - 1;
@@ -478,7 +478,7 @@ void MPEG::File::read(bool readProperties)
   ID3v1Tag(true);
 }
 
-long MPEG::File::findID3v2()
+offset_t MPEG::File::findID3v2()
 {
   if(!isValid())
     return -1;
diff --git a/taglib/mpeg/mpegfile.h b/taglib/mpeg/mpegfile.h
index e9e97387..a13dc3d6 100644
--- a/taglib/mpeg/mpegfile.h
+++ b/taglib/mpeg/mpegfile.h
@@ -330,24 +330,24 @@ namespace TagLib {
       /*!
        * Returns the position in the file of the first MPEG frame.
        */
-      long firstFrameOffset();
+      offset_t firstFrameOffset();
 
       /*!
        * Returns the position in the file of the next MPEG frame,
        * using the current position as start
        */
-      long nextFrameOffset(long position);
+      offset_t nextFrameOffset(offset_t position);
 
       /*!
        * Returns the position in the file of the previous MPEG frame,
        * using the current position as start
        */
-      long previousFrameOffset(long position);
+      offset_t previousFrameOffset(offset_t position);
 
       /*!
        * Returns the position in the file of the last MPEG frame.
        */
-      long lastFrameOffset();
+      offset_t lastFrameOffset();
 
       /*!
        * Returns whether or not the file on disk actually has an ID3v1 tag.
@@ -375,7 +375,7 @@ namespace TagLib {
       File &operator=(const File &);
 
       void read(bool readProperties);
-      long findID3v2();
+      offset_t findID3v2();
 
       class FilePrivate;
       FilePrivate *d;
diff --git a/taglib/mpeg/mpegheader.cpp b/taglib/mpeg/mpegheader.cpp
index e678f15b..492b471e 100644
--- a/taglib/mpeg/mpegheader.cpp
+++ b/taglib/mpeg/mpegheader.cpp
@@ -75,7 +75,7 @@ MPEG::Header::Header(const ByteVector &data) :
   debug("MPEG::Header::Header() - This constructor is no longer used.");
 }
 
-MPEG::Header::Header(File *file, long offset, bool checkLength) :
+MPEG::Header::Header(File *file, offset_t offset, bool checkLength) :
   d(new HeaderPrivate())
 {
   parse(file, offset, checkLength);
@@ -170,7 +170,7 @@ MPEG::Header &MPEG::Header::operator=(const Header &h)
 // private members
 ////////////////////////////////////////////////////////////////////////////////
 
-void MPEG::Header::parse(File *file, long offset, bool checkLength)
+void MPEG::Header::parse(File *file, offset_t offset, bool checkLength)
 {
   file->seek(offset);
   const ByteVector data = file->readBlock(4);
diff --git a/taglib/mpeg/mpegheader.h b/taglib/mpeg/mpegheader.h
index 024aa112..ee2cbc6e 100644
--- a/taglib/mpeg/mpegheader.h
+++ b/taglib/mpeg/mpegheader.h
@@ -61,7 +61,7 @@ namespace TagLib {
        * check if the frame length is parsed and calculated correctly.  So it's
        * suitable for seeking for the first valid frame.
        */
-      Header(File *file, long offset, bool checkLength = true);
+      Header(File *file, offset_t offset, bool checkLength = true);
 
       /*!
        * Does a shallow copy of \a h.
@@ -167,7 +167,7 @@ namespace TagLib {
       Header &operator=(const Header &h);
 
     private:
-      void parse(File *file, long offset, bool checkLength);
+      void parse(File *file, offset_t offset, bool checkLength);
 
       class HeaderPrivate;
       HeaderPrivate *d;
diff --git a/taglib/mpeg/mpegproperties.cpp b/taglib/mpeg/mpegproperties.cpp
index 6e7bb823..effc30f4 100644
--- a/taglib/mpeg/mpegproperties.cpp
+++ b/taglib/mpeg/mpegproperties.cpp
@@ -157,7 +157,7 @@ void MPEG::Properties::read(File *file)
 {
   // Only the first valid frame is required if we have a VBR header.
 
-  long firstFrameOffset = file->firstFrameOffset();
+  offset_t firstFrameOffset = file->firstFrameOffset();
   if(firstFrameOffset < 0) {
     debug("MPEG::Properties::read() -- Could not find an MPEG frame in the stream.");
     return;
@@ -207,7 +207,7 @@ void MPEG::Properties::read(File *file)
 
     // Look for the last MPEG audio frame to calculate the stream length.
 
-    long lastFrameOffset = file->lastFrameOffset();
+    offset_t lastFrameOffset = file->lastFrameOffset();
     if(lastFrameOffset < 0) {
       debug("MPEG::Properties::read() -- Could not find an MPEG frame in the stream.");
       return;
@@ -225,7 +225,7 @@ void MPEG::Properties::read(File *file)
       lastHeader = Header(file, lastFrameOffset, false);
     }
 
-    const long streamLength = lastFrameOffset - firstFrameOffset + lastHeader.frameLength();
+    const offset_t streamLength = lastFrameOffset - firstFrameOffset + lastHeader.frameLength();
     if(streamLength > 0)
       d->length = static_cast<int>(streamLength * 8.0 / d->bitrate + 0.5);
   }
diff --git a/taglib/ogg/flac/oggflacfile.cpp b/taglib/ogg/flac/oggflacfile.cpp
index 19348e6f..16e30b0f 100644
--- a/taglib/ogg/flac/oggflacfile.cpp
+++ b/taglib/ogg/flac/oggflacfile.cpp
@@ -57,8 +57,8 @@ public:
   Properties *properties;
   ByteVector streamInfoData;
   ByteVector xiphCommentData;
-  long streamStart;
-  long streamLength;
+  offset_t streamStart;
+  offset_t streamLength;
   bool scanned;
 
   bool hasXiphComment;
@@ -191,7 +191,7 @@ ByteVector Ogg::FLAC::File::xiphCommentData()
   return d->xiphCommentData;
 }
 
-long Ogg::FLAC::File::streamLength()
+offset_t Ogg::FLAC::File::streamLength()
 {
   scan();
   return d->streamLength;
@@ -208,7 +208,7 @@ void Ogg::FLAC::File::scan()
     return;
 
   int ipacket = 0;
-  long overhead = 0;
+  offset_t overhead = 0;
 
   ByteVector metadataHeader = packet(ipacket);
   if(metadataHeader.isEmpty())
diff --git a/taglib/ogg/flac/oggflacfile.h b/taglib/ogg/flac/oggflacfile.h
index 28b3f67f..e4daf370 100644
--- a/taglib/ogg/flac/oggflacfile.h
+++ b/taglib/ogg/flac/oggflacfile.h
@@ -137,7 +137,7 @@ namespace TagLib {
        * Returns the length of the audio-stream, used by FLAC::Properties for
        * calculating the bitrate.
        */
-      long streamLength();
+      offset_t streamLength();
 
       /*!
        * Returns whether or not the file on disk actually has a XiphComment.
diff --git a/taglib/ogg/oggfile.cpp b/taglib/ogg/oggfile.cpp
index c36e4d46..aa284a3b 100644
--- a/taglib/ogg/oggfile.cpp
+++ b/taglib/ogg/oggfile.cpp
@@ -130,7 +130,7 @@ void Ogg::File::setPacket(unsigned int i, const ByteVector &p)
 const Ogg::PageHeader *Ogg::File::firstPageHeader()
 {
   if(!d->firstPageHeader) {
-    const long firstPageHeaderOffset = find("OggS");
+    const offset_t firstPageHeaderOffset = find("OggS");
     if(firstPageHeaderOffset < 0)
       return 0;
 
@@ -143,7 +143,7 @@ const Ogg::PageHeader *Ogg::File::firstPageHeader()
 const Ogg::PageHeader *Ogg::File::lastPageHeader()
 {
   if(!d->lastPageHeader) {
-    const long lastPageHeaderOffset = rfind("OggS");
+    const offset_t lastPageHeaderOffset = rfind("OggS");
     if(lastPageHeaderOffset < 0)
       return 0;
 
@@ -193,7 +193,7 @@ bool Ogg::File::readPages(unsigned int i)
 {
   while(true) {
     unsigned int packetIndex;
-    long offset;
+    offset_t offset;
 
     if(d->pages.isEmpty()) {
       packetIndex = 0;
@@ -276,8 +276,8 @@ void Ogg::File::writePacket(unsigned int i, const ByteVector &packet)
   for(it = pages.begin(); it != pages.end(); ++it)
     data.append((*it)->render());
 
-  const unsigned long originalOffset = firstPage->fileOffset();
-  const unsigned long originalLength = lastPage->fileOffset() + lastPage->size() - originalOffset;
+  const offset_t originalOffset = firstPage->fileOffset();
+  const offset_t originalLength = lastPage->fileOffset() + lastPage->size() - originalOffset;
 
   insert(data, originalOffset, originalLength);
 
diff --git a/taglib/ogg/oggpage.cpp b/taglib/ogg/oggpage.cpp
index 75aea22a..e5fd70d2 100644
--- a/taglib/ogg/oggpage.cpp
+++ b/taglib/ogg/oggpage.cpp
@@ -37,14 +37,14 @@ using namespace TagLib;
 class Ogg::Page::PagePrivate
 {
 public:
-  PagePrivate(File *f = 0, long pageOffset = -1) :
+  PagePrivate(File *f = 0, offset_t pageOffset = -1) :
     file(f),
     fileOffset(pageOffset),
     header(f, pageOffset),
     firstPacketIndex(-1) {}
 
   File *file;
-  long fileOffset;
+  offset_t fileOffset;
   PageHeader header;
   int firstPacketIndex;
   ByteVectorList packets;
@@ -54,7 +54,7 @@ public:
 // public members
 ////////////////////////////////////////////////////////////////////////////////
 
-Ogg::Page::Page(Ogg::File *file, long pageOffset) :
+Ogg::Page::Page(Ogg::File *file, offset_t pageOffset) :
   d(new PagePrivate(file, pageOffset))
 {
 }
@@ -64,7 +64,7 @@ Ogg::Page::~Page()
   delete d;
 }
 
-long Ogg::Page::fileOffset() const
+offset_t Ogg::Page::fileOffset() const
 {
   return d->fileOffset;
 }
diff --git a/taglib/ogg/oggpage.h b/taglib/ogg/oggpage.h
index 13e3e7f9..e5a2f362 100644
--- a/taglib/ogg/oggpage.h
+++ b/taglib/ogg/oggpage.h
@@ -55,14 +55,14 @@ namespace TagLib {
       /*!
        * Read an Ogg page from the \a file at the position \a pageOffset.
        */
-      Page(File *file, long pageOffset);
+      Page(File *file, offset_t pageOffset);
 
       virtual ~Page();
 
       /*!
        * Returns the page's position within the file (in bytes).
        */
-      long fileOffset() const;
+      offset_t fileOffset() const;
 
       /*!
        * Returns a pointer to the header for this page.  This pointer will become
diff --git a/taglib/ogg/oggpageheader.cpp b/taglib/ogg/oggpageheader.cpp
index b867567c..cc645eaf 100644
--- a/taglib/ogg/oggpageheader.cpp
+++ b/taglib/ogg/oggpageheader.cpp
@@ -66,7 +66,7 @@ public:
 // public members
 ////////////////////////////////////////////////////////////////////////////////
 
-Ogg::PageHeader::PageHeader(Ogg::File *file, long pageOffset) :
+Ogg::PageHeader::PageHeader(Ogg::File *file, offset_t pageOffset) :
   d(new PageHeaderPrivate())
 {
   if(file && pageOffset >= 0)
@@ -225,7 +225,7 @@ ByteVector Ogg::PageHeader::render() const
 // private members
 ////////////////////////////////////////////////////////////////////////////////
 
-void Ogg::PageHeader::read(Ogg::File *file, long pageOffset)
+void Ogg::PageHeader::read(Ogg::File *file, offset_t pageOffset)
 {
   file->seek(pageOffset);
 
diff --git a/taglib/ogg/oggpageheader.h b/taglib/ogg/oggpageheader.h
index 42f67307..000216c1 100644
--- a/taglib/ogg/oggpageheader.h
+++ b/taglib/ogg/oggpageheader.h
@@ -52,7 +52,7 @@ namespace TagLib {
        * create a page with no (and as such, invalid) data that must be set
        * later.
        */
-      PageHeader(File *file = 0, long pageOffset = -1);
+      PageHeader(File *file = 0, offset_t pageOffset = -1);
 
       /*!
        * Deletes this instance of the PageHeader.
@@ -219,7 +219,7 @@ namespace TagLib {
       PageHeader(const PageHeader &);
       PageHeader &operator=(const PageHeader &);
 
-      void read(Ogg::File *file, long pageOffset);
+      void read(Ogg::File *file, offset_t pageOffset);
       ByteVector lacingValues() const;
 
       class PageHeaderPrivate;
diff --git a/taglib/ogg/opus/opusproperties.cpp b/taglib/ogg/opus/opusproperties.cpp
index b60cc01d..e19ab64d 100644
--- a/taglib/ogg/opus/opusproperties.cpp
+++ b/taglib/ogg/opus/opusproperties.cpp
@@ -163,7 +163,7 @@ void Opus::Properties::read(File *file)
 
       if(frameCount > 0) {
         const double length = frameCount * 1000.0 / 48000.0;
-        long fileLengthWithoutOverhead = file->length();
+        offset_t fileLengthWithoutOverhead = file->length();
         // Ignore the two mandatory header packets, see "3. Packet Organization"
         // in https://tools.ietf.org/html/rfc7845.html
         for (unsigned int i = 0; i < 2; ++i) {
diff --git a/taglib/ogg/speex/speexproperties.cpp b/taglib/ogg/speex/speexproperties.cpp
index b7a11cc6..fae184a0 100644
--- a/taglib/ogg/speex/speexproperties.cpp
+++ b/taglib/ogg/speex/speexproperties.cpp
@@ -182,7 +182,7 @@ void Speex::Properties::read(File *file)
 
       if(frameCount > 0) {
         const double length = frameCount * 1000.0 / d->sampleRate;
-        long fileLengthWithoutOverhead = file->length();
+        offset_t fileLengthWithoutOverhead = file->length();
         // Ignore the two header packets, see "Ogg file format" in
         // https://www.speex.org/docs/manual/speex-manual/node8.html
         for (unsigned int i = 0; i < 2; ++i) {
diff --git a/taglib/ogg/vorbis/vorbisproperties.cpp b/taglib/ogg/vorbis/vorbisproperties.cpp
index 4000c254..6f6c8907 100644
--- a/taglib/ogg/vorbis/vorbisproperties.cpp
+++ b/taglib/ogg/vorbis/vorbisproperties.cpp
@@ -186,7 +186,7 @@ void Vorbis::Properties::read(File *file)
 
       if(frameCount > 0) {
         const double length = frameCount * 1000.0 / d->sampleRate;
-        long fileLengthWithoutOverhead = file->length();
+        offset_t fileLengthWithoutOverhead = file->length();
         // Ignore the three initial header packets, see "1.3.1. Decode Setup" in
         // https://xiph.org/vorbis/doc/Vorbis_I_spec.html
         for (unsigned int i = 0; i < 3; ++i) {
diff --git a/taglib/riff/rifffile.cpp b/taglib/riff/rifffile.cpp
index 930323d6..56f8b7a7 100644
--- a/taglib/riff/rifffile.cpp
+++ b/taglib/riff/rifffile.cpp
@@ -38,7 +38,7 @@ using namespace TagLib;
 struct Chunk
 {
   ByteVector   name;
-  unsigned int offset;
+  offset_t offset;
   unsigned int size;
   unsigned int padding;
 };
@@ -54,7 +54,7 @@ public:
   const Endianness endianness;
 
   unsigned int size;
-  long sizeOffset;
+  offset_t sizeOffset;
 
   std::vector<Chunk> chunks;
 };
@@ -108,7 +108,7 @@ unsigned int RIFF::File::chunkDataSize(unsigned int i) const
   return d->chunks[i].size;
 }
 
-unsigned int RIFF::File::chunkOffset(unsigned int i) const
+offset_t RIFF::File::chunkOffset(unsigned int i) const
 {
   if(i >= d->chunks.size()) {
     debug("RIFF::File::chunkPadding() - Index out of range. Returning 0.");
@@ -212,7 +212,7 @@ void RIFF::File::setChunkData(const ByteVector &name, const ByteVector &data, bo
 
   Chunk &last = d->chunks.back();
 
-  long offset = last.offset + last.size + last.padding;
+  offset_t offset = last.offset + last.size + last.padding;
   if(offset & 1) {
     if(last.padding == 1) {
       last.padding = 0; // This should not happen unless the file is corrupted.
@@ -283,7 +283,7 @@ void RIFF::File::read()
 {
   const bool bigEndian = (d->endianness == BigEndian);
 
-  long offset = tell();
+  offset_t offset = tell();
 
   offset += 4;
   d->sizeOffset = offset;
@@ -352,7 +352,7 @@ void RIFF::File::read()
 }
 
 void RIFF::File::writeChunk(const ByteVector &name, const ByteVector &data,
-                            unsigned long offset, unsigned long replace)
+                            offset_t offset, unsigned long replace)
 {
   ByteVector combined;
 
diff --git a/taglib/riff/rifffile.h b/taglib/riff/rifffile.h
index 5c606b4a..cf821baf 100644
--- a/taglib/riff/rifffile.h
+++ b/taglib/riff/rifffile.h
@@ -71,7 +71,7 @@ namespace TagLib {
       /*!
        * \return The offset within the file for the selected chunk number.
        */
-      unsigned int chunkOffset(unsigned int i) const;
+      offset_t chunkOffset(unsigned int i) const;
 
       /*!
        * \return The size of the chunk data.
@@ -145,7 +145,7 @@ namespace TagLib {
 
       void read();
       void writeChunk(const ByteVector &name, const ByteVector &data,
-                      unsigned long offset, unsigned long replace = 0);
+                      offset_t offset, unsigned long replace = 0);
 
       /*!
        * Update the global RIFF size based on the current internal structure.
diff --git a/taglib/tagutils.cpp b/taglib/tagutils.cpp
index dc047040..2c556ca7 100644
--- a/taglib/tagutils.cpp
+++ b/taglib/tagutils.cpp
@@ -33,13 +33,13 @@
 
 using namespace TagLib;
 
-long Utils::findID3v1(File *file)
+offset_t Utils::findID3v1(File *file)
 {
   if(!file->isValid())
     return -1;
 
   file->seek(-128, File::End);
-  const long p = file->tell();
+  const offset_t p = file->tell();
 
   if(file->readBlock(3) == ID3v1::Tag::fileIdentifier())
     return p;
@@ -47,7 +47,7 @@ long Utils::findID3v1(File *file)
   return -1;
 }
 
-long Utils::findID3v2(File *file)
+offset_t Utils::findID3v2(File *file)
 {
   if(!file->isValid())
     return -1;
@@ -60,7 +60,7 @@ long Utils::findID3v2(File *file)
   return -1;
 }
 
-long Utils::findAPE(File *file, long id3v1Location)
+offset_t Utils::findAPE(File *file, offset_t id3v1Location)
 {
   if(!file->isValid())
     return -1;
@@ -70,7 +70,7 @@ long Utils::findAPE(File *file, long id3v1Location)
   else
     file->seek(-32, File::End);
 
-  const long p = file->tell();
+  const offset_t p = file->tell();
 
   if(file->readBlock(8) == APE::Tag::fileIdentifier())
     return p;
diff --git a/taglib/tagutils.h b/taglib/tagutils.h
index fb11d1e0..0001f34b 100644
--- a/taglib/tagutils.h
+++ b/taglib/tagutils.h
@@ -36,11 +36,11 @@ namespace TagLib {
 
   namespace Utils {
 
-    long findID3v1(File *file);
+    offset_t findID3v1(File *file);
 
-    long findID3v2(File *file);
+    offset_t findID3v2(File *file);
 
-    long findAPE(File *file, long id3v1Location);
+    offset_t findAPE(File *file, offset_t id3v1Location);
   }
 }
 
diff --git a/taglib/toolkit/taglib.h b/taglib/toolkit/taglib.h
index f2e7a9de..31c1ae59 100644
--- a/taglib/toolkit/taglib.h
+++ b/taglib/toolkit/taglib.h
@@ -44,6 +44,8 @@
 #define TAGLIB_CONSTRUCT_BITSET(x) static_cast<unsigned long>(x)
 #endif
 
+#define TAGLIB_WITH_OFFSET_TYPE
+
 #include <string>
 
 //! A namespace for all TagLib related classes and functions
@@ -69,6 +71,14 @@ namespace TagLib {
   typedef unsigned long      ulong;
   typedef unsigned long long ulonglong;
 
+  // Offset or length type for I/O streams.
+  // In Win32, always 64bit. Otherwise, equivalent to off_t.
+#ifdef _WIN32
+  typedef long long offset_t;
+#else
+  typedef off_t     offset_t;
+#endif
+
   /*!
    * Unfortunately std::wstring isn't defined on some systems, (i.e. GCC < 3)
    * so I'm providing something here that should be constant.
diff --git a/taglib/toolkit/tbytevectorstream.cpp b/taglib/toolkit/tbytevectorstream.cpp
index 74b2eced..d773791f 100644
--- a/taglib/toolkit/tbytevectorstream.cpp
+++ b/taglib/toolkit/tbytevectorstream.cpp
@@ -40,7 +40,7 @@ public:
   ByteVectorStreamPrivate(const ByteVector &data);
 
   ByteVector data;
-  long position;
+  offset_t position;
 };
 
 ByteVectorStream::ByteVectorStreamPrivate::ByteVectorStreamPrivate(const ByteVector &data) :
@@ -88,7 +88,7 @@ void ByteVectorStream::writeBlock(const ByteVector &data)
   d->position += size;
 }
 
-void ByteVectorStream::insert(const ByteVector &data, unsigned long start, unsigned long replace)
+void ByteVectorStream::insert(const ByteVector &data, offset_t start, unsigned long replace)
 {
   long sizeDiff = data.size() - replace;
   if(sizeDiff < 0) {
@@ -96,18 +96,18 @@ void ByteVectorStream::insert(const ByteVector &data, unsigned long start, unsig
   }
   else if(sizeDiff > 0) {
     truncate(length() + sizeDiff);
-    unsigned long readPosition  = start + replace;
-    unsigned long writePosition = start + data.size();
+    offset_t readPosition  = start + replace;
+    offset_t writePosition = start + data.size();
     memmove(d->data.data() + writePosition, d->data.data() + readPosition, length() - sizeDiff - readPosition);
   }
   seek(start);
   writeBlock(data);
 }
 
-void ByteVectorStream::removeBlock(unsigned long start, unsigned long length)
+void ByteVectorStream::removeBlock(offset_t start, unsigned long length)
 {
-  unsigned long readPosition = start + length;
-  unsigned long writePosition = start;
+  offset_t readPosition = start + length;
+  offset_t writePosition = start;
   if(readPosition < static_cast<unsigned long>(ByteVectorStream::length())) {
     unsigned long bytesToMove = ByteVectorStream::length() - readPosition;
     memmove(d->data.data() + writePosition, d->data.data() + readPosition, bytesToMove);
@@ -127,7 +127,7 @@ bool ByteVectorStream::isOpen() const
   return true;
 }
 
-void ByteVectorStream::seek(long offset, Position p)
+void ByteVectorStream::seek(offset_t offset, Position p)
 {
   switch(p) {
   case Beginning:
@@ -146,17 +146,17 @@ void ByteVectorStream::clear()
 {
 }
 
-long ByteVectorStream::tell() const
+offset_t ByteVectorStream::tell() const
 {
   return d->position;
 }
 
-long ByteVectorStream::length()
+offset_t ByteVectorStream::length()
 {
   return d->data.size();
 }
 
-void ByteVectorStream::truncate(long length)
+void ByteVectorStream::truncate(offset_t length)
 {
   d->data.resize(length);
 }
diff --git a/taglib/toolkit/tbytevectorstream.h b/taglib/toolkit/tbytevectorstream.h
index 84327c46..78ebfd4a 100644
--- a/taglib/toolkit/tbytevectorstream.h
+++ b/taglib/toolkit/tbytevectorstream.h
@@ -81,7 +81,7 @@ namespace TagLib {
      * \note This method is slow since it requires rewriting all of the file
      * after the insertion point.
      */
-    void insert(const ByteVector &data, unsigned long start = 0, unsigned long replace = 0);
+    void insert(const ByteVector &data, offset_t start = 0, unsigned long replace = 0);
 
     /*!
      * Removes a block of the file starting a \a start and continuing for
@@ -90,7 +90,7 @@ namespace TagLib {
      * \note This method is slow since it involves rewriting all of the file
      * after the removed portion.
      */
-    void removeBlock(unsigned long start = 0, unsigned long length = 0);
+    void removeBlock(offset_t start = 0, unsigned long length = 0);
 
     /*!
      * Returns true if the file is read only (or if the file can not be opened).
@@ -109,7 +109,7 @@ namespace TagLib {
      *
      * \see Position
      */
-    void seek(long offset, Position p = Beginning);
+    void seek(offset_t offset, Position p = Beginning);
 
     /*!
      * Reset the end-of-file and error flags on the file.
@@ -119,17 +119,17 @@ namespace TagLib {
     /*!
      * Returns the current offset within the file.
      */
-    long tell() const;
+    offset_t tell() const;
 
     /*!
      * Returns the length of the file.
      */
-    long length();
+    offset_t length();
 
     /*!
      * Truncates the file to a \a length.
      */
-    void truncate(long length);
+    void truncate(offset_t length);
 
     ByteVector *data();
 
diff --git a/taglib/toolkit/tfile.cpp b/taglib/toolkit/tfile.cpp
index c634baa8..93dc8cae 100644
--- a/taglib/toolkit/tfile.cpp
+++ b/taglib/toolkit/tfile.cpp
@@ -234,14 +234,14 @@ void File::writeBlock(const ByteVector &data)
   d->stream->writeBlock(data);
 }
 
-long File::find(const ByteVector &pattern, long fromOffset, const ByteVector &before)
+offset_t File::find(const ByteVector &pattern, offset_t fromOffset, const ByteVector &before)
 {
   if(!d->stream || pattern.size() > bufferSize())
       return -1;
 
   // The position in the file that the current buffer starts at.
 
-  long bufferOffset = fromOffset;
+  offset_t bufferOffset = fromOffset;
   ByteVector buffer;
 
   // These variables are used to keep track of a partial match that happens at
@@ -253,7 +253,7 @@ long File::find(const ByteVector &pattern, long fromOffset, const ByteVector &be
   // Save the location of the current read pointer.  We will restore the
   // position using seek() before all returns.
 
-  long originalPosition = tell();
+  offset_t originalPosition = tell();
 
   // Start the search at the offset.
 
@@ -330,7 +330,7 @@ long File::find(const ByteVector &pattern, long fromOffset, const ByteVector &be
 }
 
 
-long File::rfind(const ByteVector &pattern, long fromOffset, const ByteVector &before)
+offset_t File::rfind(const ByteVector &pattern, offset_t fromOffset, const ByteVector &before)
 {
   if(!d->stream || pattern.size() > bufferSize())
       return -1;
@@ -350,7 +350,7 @@ long File::rfind(const ByteVector &pattern, long fromOffset, const ByteVector &b
   // Save the location of the current read pointer.  We will restore the
   // position using seek() before all returns.
 
-  long originalPosition = tell();
+  offset_t originalPosition = tell();
 
   // Start the search at the offset.
 
@@ -358,7 +358,7 @@ long File::rfind(const ByteVector &pattern, long fromOffset, const ByteVector &b
     fromOffset = length();
 
   long bufferLength = bufferSize();
-  long bufferOffset = fromOffset + pattern.size();
+  offset_t bufferOffset = fromOffset + pattern.size();
 
   // See the notes in find() for an explanation of this algorithm.
 
@@ -404,12 +404,12 @@ long File::rfind(const ByteVector &pattern, long fromOffset, const ByteVector &b
   return -1;
 }
 
-void File::insert(const ByteVector &data, unsigned long start, unsigned long replace)
+void File::insert(const ByteVector &data, offset_t start, unsigned long replace)
 {
   d->stream->insert(data, start, replace);
 }
 
-void File::removeBlock(unsigned long start, unsigned long length)
+void File::removeBlock(offset_t start, unsigned long length)
 {
   d->stream->removeBlock(start, length);
 }
@@ -429,12 +429,12 @@ bool File::isValid() const
   return isOpen() && d->valid;
 }
 
-void File::seek(long offset, Position p)
+void File::seek(offset_t offset, Position p)
 {
   d->stream->seek(offset, IOStream::Position(p));
 }
 
-void File::truncate(long length)
+void File::truncate(offset_t length)
 {
   d->stream->truncate(length);
 }
@@ -444,12 +444,12 @@ void File::clear()
   d->stream->clear();
 }
 
-long File::tell() const
+offset_t File::tell() const
 {
   return d->stream->tell();
 }
 
-long File::length()
+offset_t File::length()
 {
   return d->stream->length();
 }
diff --git a/taglib/toolkit/tfile.h b/taglib/toolkit/tfile.h
index a6dda7ba..0840a123 100644
--- a/taglib/toolkit/tfile.h
+++ b/taglib/toolkit/tfile.h
@@ -163,8 +163,8 @@ namespace TagLib {
      * \note This has the practical limitation that \a pattern can not be longer
      * than the buffer size used by readBlock().  Currently this is 1024 bytes.
      */
-    long find(const ByteVector &pattern,
-              long fromOffset = 0,
+    offset_t find(const ByteVector &pattern,
+              offset_t fromOffset = 0,
               const ByteVector &before = ByteVector());
 
     /*!
@@ -179,8 +179,8 @@ namespace TagLib {
      * \note This has the practical limitation that \a pattern can not be longer
      * than the buffer size used by readBlock().  Currently this is 1024 bytes.
      */
-    long rfind(const ByteVector &pattern,
-               long fromOffset = 0,
+    offset_t rfind(const ByteVector &pattern,
+               offset_t fromOffset = 0,
                const ByteVector &before = ByteVector());
 
     /*!
@@ -190,7 +190,7 @@ namespace TagLib {
      * \note This method is slow since it requires rewriting all of the file
      * after the insertion point.
      */
-    void insert(const ByteVector &data, unsigned long start = 0, unsigned long replace = 0);
+    void insert(const ByteVector &data, offset_t start = 0, unsigned long replace = 0);
 
     /*!
      * Removes a block of the file starting a \a start and continuing for
@@ -199,7 +199,7 @@ namespace TagLib {
      * \note This method is slow since it involves rewriting all of the file
      * after the removed portion.
      */
-    void removeBlock(unsigned long start = 0, unsigned long length = 0);
+    void removeBlock(offset_t start = 0, unsigned long length = 0);
 
     /*!
      * Returns true if the file is read only (or if the file can not be opened).
@@ -223,7 +223,7 @@ namespace TagLib {
      *
      * \see Position
      */
-    void seek(long offset, Position p = Beginning);
+    void seek(offset_t offset, Position p = Beginning);
 
     /*!
      * Reset the end-of-file and error flags on the file.
@@ -233,12 +233,12 @@ namespace TagLib {
     /*!
      * Returns the current offset within the file.
      */
-    long tell() const;
+    offset_t tell() const;
 
     /*!
      * Returns the length of the file.
      */
-    long length();
+    offset_t length();
 
     /*!
      * Returns true if \a file can be opened for reading.  If the file does not
@@ -286,7 +286,7 @@ namespace TagLib {
     /*!
      * Truncates the file to a \a length.
      */
-    void truncate(long length);
+    void truncate(offset_t length);
 
     /*!
      * Returns the buffer size that is used for internal buffering.
diff --git a/taglib/toolkit/tfilestream.cpp b/taglib/toolkit/tfilestream.cpp
index 5205bae0..38c5ed83 100644
--- a/taglib/toolkit/tfilestream.cpp
+++ b/taglib/toolkit/tfilestream.cpp
@@ -209,7 +209,7 @@ void FileStream::writeBlock(const ByteVector &data)
   writeFile(d->file, data);
 }
 
-void FileStream::insert(const ByteVector &data, unsigned long start, unsigned long replace)
+void FileStream::insert(const ByteVector &data, offset_t start, unsigned long replace)
 {
   if(!isOpen()) {
     debug("FileStream::insert() -- invalid file.");
@@ -243,15 +243,15 @@ void FileStream::insert(const ByteVector &data, unsigned long start, unsigned lo
   // the *differnce* in the tag sizes.  We want to avoid overwriting parts
   // that aren't yet in memory, so this is necessary.
 
-  unsigned long bufferLength = bufferSize();
+  size_t bufferLength = bufferSize();
 
   while(data.size() - replace > bufferLength)
     bufferLength += bufferSize();
 
   // Set where to start the reading and writing.
 
-  long readPosition = start + replace;
-  long writePosition = start;
+  offset_t readPosition = start + replace;
+  offset_t writePosition = start;
 
   ByteVector buffer = data;
   ByteVector aboutToOverwrite(static_cast<unsigned int>(bufferLength));
@@ -291,7 +291,7 @@ void FileStream::insert(const ByteVector &data, unsigned long start, unsigned lo
   }
 }
 
-void FileStream::removeBlock(unsigned long start, unsigned long length)
+void FileStream::removeBlock(offset_t start, unsigned long length)
 {
   if(!isOpen()) {
     debug("FileStream::removeBlock() -- invalid file.");
@@ -300,8 +300,8 @@ void FileStream::removeBlock(unsigned long start, unsigned long length)
 
   unsigned long bufferLength = bufferSize();
 
-  long readPosition = start + length;
-  long writePosition = start;
+  offset_t readPosition = start + length;
+  offset_t writePosition = start;
 
   ByteVector buffer(static_cast<unsigned int>(bufferLength));
 
@@ -338,7 +338,7 @@ bool FileStream::isOpen() const
   return (d->file != InvalidFileHandle);
 }
 
-void FileStream::seek(long offset, Position p)
+void FileStream::seek(offset_t offset, Position p)
 {
   if(!isOpen()) {
     debug("FileStream::seek() -- invalid file.");
@@ -364,7 +364,9 @@ void FileStream::seek(long offset, Position p)
   }
 
   SetLastError(NO_ERROR);
-  SetFilePointer(d->file, offset, NULL, whence);
+  LARGE_INTEGER largeOffset = {};
+  largeOffset.QuadPart = offset;
+  SetFilePointerEx(d->file, largeOffset, NULL, whence);
 
   const int lastError = GetLastError();
   if(lastError != NO_ERROR && lastError != ERROR_NEGATIVE_SEEK)
@@ -406,14 +408,16 @@ void FileStream::clear()
 #endif
 }
 
-long FileStream::tell() const
+offset_t FileStream::tell() const
 {
 #ifdef _WIN32
 
   SetLastError(NO_ERROR);
-  const DWORD position = SetFilePointer(d->file, 0, NULL, FILE_CURRENT);
+  LARGE_INTEGER largeOffset = {};
+  LARGE_INTEGER newPointer;
+  SetFilePointerEx(d->file, largeOffset, &newPointer, FILE_CURRENT);
   if(GetLastError() == NO_ERROR) {
-    return static_cast<long>(position);
+    return newPointer.QuadPart;
   }
   else {
     debug("FileStream::tell() -- Failed to get the file pointer.");
@@ -427,7 +431,7 @@ long FileStream::tell() const
 #endif
 }
 
-long FileStream::length()
+offset_t FileStream::length()
 {
   if(!isOpen()) {
     debug("FileStream::length() -- invalid file.");
@@ -448,10 +452,10 @@ long FileStream::length()
 
 #else
 
-  const long curpos = tell();
+  const offset_t curpos = tell();
 
   seek(0, End);
-  const long endpos = tell();
+  const offset_t endpos = tell();
 
   seek(curpos, Beginning);
 
@@ -464,11 +468,11 @@ long FileStream::length()
 // protected members
 ////////////////////////////////////////////////////////////////////////////////
 
-void FileStream::truncate(long length)
+void FileStream::truncate(offset_t length)
 {
 #ifdef _WIN32
 
-  const long currentPos = tell();
+  const offset_t currentPos = tell();
 
   seek(length);
 
diff --git a/taglib/toolkit/tfilestream.h b/taglib/toolkit/tfilestream.h
index 96a476d6..e594b801 100644
--- a/taglib/toolkit/tfilestream.h
+++ b/taglib/toolkit/tfilestream.h
@@ -87,7 +87,7 @@ namespace TagLib {
      * \note This method is slow since it requires rewriting all of the file
      * after the insertion point.
      */
-    void insert(const ByteVector &data, unsigned long start = 0, unsigned long replace = 0);
+    void insert(const ByteVector &data, offset_t start = 0, unsigned long replace = 0);
 
     /*!
      * Removes a block of the file starting a \a start and continuing for
@@ -96,7 +96,7 @@ namespace TagLib {
      * \note This method is slow since it involves rewriting all of the file
      * after the removed portion.
      */
-    void removeBlock(unsigned long start = 0, unsigned long length = 0);
+    void removeBlock(offset_t start = 0, unsigned long length = 0);
 
     /*!
      * Returns true if the file is read only (or if the file can not be opened).
@@ -115,7 +115,7 @@ namespace TagLib {
      *
      * \see Position
      */
-    void seek(long offset, Position p = Beginning);
+    void seek(offset_t offset, Position p = Beginning);
 
     /*!
      * Reset the end-of-file and error flags on the file.
@@ -125,17 +125,17 @@ namespace TagLib {
     /*!
      * Returns the current offset within the file.
      */
-    long tell() const;
+    offset_t tell() const;
 
     /*!
      * Returns the length of the file.
      */
-    long length();
+    offset_t length();
 
     /*!
      * Truncates the file to a \a length.
      */
-    void truncate(long length);
+    void truncate(offset_t length);
 
   protected:
 
diff --git a/taglib/toolkit/tiostream.h b/taglib/toolkit/tiostream.h
index 11053164..25417fce 100644
--- a/taglib/toolkit/tiostream.h
+++ b/taglib/toolkit/tiostream.h
@@ -110,7 +110,7 @@ namespace TagLib {
      * after the insertion point.
      */
     virtual void insert(const ByteVector &data,
-                        unsigned long start = 0, unsigned long replace = 0) = 0;
+                        offset_t start = 0, unsigned long replace = 0) = 0;
 
     /*!
      * Removes a block of the file starting a \a start and continuing for
@@ -119,7 +119,7 @@ namespace TagLib {
      * \note This method is slow since it involves rewriting all of the file
      * after the removed portion.
      */
-    virtual void removeBlock(unsigned long start = 0, unsigned long length = 0) = 0;
+    virtual void removeBlock(offset_t start = 0, unsigned long length = 0) = 0;
 
     /*!
      * Returns true if the file is read only (or if the file can not be opened).
@@ -138,7 +138,7 @@ namespace TagLib {
      *
      * \see Position
      */
-    virtual void seek(long offset, Position p = Beginning) = 0;
+    virtual void seek(offset_t offset, Position p = Beginning) = 0;
 
     /*!
      * Reset the end-of-stream and error flags on the stream.
@@ -148,17 +148,17 @@ namespace TagLib {
     /*!
      * Returns the current offset within the stream.
      */
-    virtual long tell() const = 0;
+    virtual offset_t tell() const = 0;
 
     /*!
      * Returns the length of the stream.
      */
-    virtual long length() = 0;
+    virtual offset_t length() = 0;
 
     /*!
      * Truncates the stream to a \a length.
      */
-    virtual void truncate(long length) = 0;
+    virtual void truncate(offset_t length) = 0;
 
   private:
     IOStream(const IOStream &);
diff --git a/taglib/trueaudio/trueaudiofile.cpp b/taglib/trueaudio/trueaudiofile.cpp
index fc123ba3..95af5567 100644
--- a/taglib/trueaudio/trueaudiofile.cpp
+++ b/taglib/trueaudio/trueaudiofile.cpp
@@ -63,10 +63,10 @@ public:
   }
 
   const ID3v2::FrameFactory *ID3v2FrameFactory;
-  long ID3v2Location;
+  offset_t ID3v2Location;
   long ID3v2OriginalSize;
 
-  long ID3v1Location;
+  offset_t ID3v1Location;
 
   TagUnion tag;
 
@@ -278,7 +278,7 @@ void TrueAudio::File::read(bool readProperties)
 
   if(readProperties) {
 
-    long streamLength;
+    offset_t streamLength;
 
     if(d->ID3v1Location >= 0)
       streamLength = d->ID3v1Location;
diff --git a/taglib/trueaudio/trueaudioproperties.cpp b/taglib/trueaudio/trueaudioproperties.cpp
index 0aab2419..3a403848 100644
--- a/taglib/trueaudio/trueaudioproperties.cpp
+++ b/taglib/trueaudio/trueaudioproperties.cpp
@@ -61,7 +61,7 @@ public:
 // public members
 ////////////////////////////////////////////////////////////////////////////////
 
-TrueAudio::Properties::Properties(const ByteVector &data, long streamLength, ReadStyle style) :
+TrueAudio::Properties::Properties(const ByteVector &data, offset_t streamLength, ReadStyle style) :
   AudioProperties(style),
   d(new PropertiesPrivate())
 {
@@ -122,7 +122,7 @@ int TrueAudio::Properties::ttaVersion() const
 // private members
 ////////////////////////////////////////////////////////////////////////////////
 
-void TrueAudio::Properties::read(const ByteVector &data, long streamLength)
+void TrueAudio::Properties::read(const ByteVector &data, offset_t streamLength)
 {
   if(data.size() < 4) {
     debug("TrueAudio::Properties::read() -- data is too short.");
diff --git a/taglib/trueaudio/trueaudioproperties.h b/taglib/trueaudio/trueaudioproperties.h
index 8dfcf375..6539b45c 100644
--- a/taglib/trueaudio/trueaudioproperties.h
+++ b/taglib/trueaudio/trueaudioproperties.h
@@ -54,7 +54,7 @@ namespace TagLib {
        * Create an instance of TrueAudio::Properties with the data read from the
        * ByteVector \a data.
        */
-      Properties(const ByteVector &data, long streamLength, ReadStyle style = Average);
+      Properties(const ByteVector &data, offset_t streamLength, ReadStyle style = Average);
 
       /*!
        * Destroys this TrueAudio::Properties instance.
@@ -122,7 +122,7 @@ namespace TagLib {
       Properties(const Properties &);
       Properties &operator=(const Properties &);
 
-      void read(const ByteVector &data, long streamLength);
+      void read(const ByteVector &data, offset_t streamLength);
 
       class PropertiesPrivate;
       PropertiesPrivate *d;
diff --git a/taglib/wavpack/wavpackfile.cpp b/taglib/wavpack/wavpackfile.cpp
index ef92f4bd..dc708f74 100644
--- a/taglib/wavpack/wavpackfile.cpp
+++ b/taglib/wavpack/wavpackfile.cpp
@@ -61,10 +61,10 @@ public:
     delete properties;
   }
 
-  long APELocation;
+  offset_t APELocation;
   long APESize;
 
-  long ID3v1Location;
+  offset_t ID3v1Location;
 
   TagUnion tag;
 
@@ -258,7 +258,7 @@ void WavPack::File::read(bool readProperties)
 
   if(readProperties) {
 
-    long streamLength;
+    offset_t streamLength;
 
     if(d->APELocation >= 0)
       streamLength = d->APELocation;
diff --git a/taglib/wavpack/wavpackproperties.cpp b/taglib/wavpack/wavpackproperties.cpp
index c1d04fd2..81597580 100644
--- a/taglib/wavpack/wavpackproperties.cpp
+++ b/taglib/wavpack/wavpackproperties.cpp
@@ -65,14 +65,14 @@ public:
 // public members
 ////////////////////////////////////////////////////////////////////////////////
 
-WavPack::Properties::Properties(const ByteVector &, long, ReadStyle style) :
+WavPack::Properties::Properties(const ByteVector &, offset_t, ReadStyle style) :
   AudioProperties(style),
   d(new PropertiesPrivate())
 {
   debug("WavPack::Properties::Properties() -- This constructor is no longer used.");
 }
 
-WavPack::Properties::Properties(File *file, long streamLength, ReadStyle style) :
+WavPack::Properties::Properties(File *file, offset_t streamLength, ReadStyle style) :
   AudioProperties(style),
   d(new PropertiesPrivate())
 {
@@ -160,9 +160,9 @@ namespace
 
 #define FINAL_BLOCK     0x1000
 
-void WavPack::Properties::read(File *file, long streamLength)
+void WavPack::Properties::read(File *file, offset_t streamLength)
 {
-  long offset = 0;
+  offset_t offset = 0;
 
   while(true) {
     file->seek(offset);
@@ -210,9 +210,9 @@ void WavPack::Properties::read(File *file, long streamLength)
   }
 }
 
-unsigned int WavPack::Properties::seekFinalIndex(File *file, long streamLength)
+unsigned int WavPack::Properties::seekFinalIndex(File *file, offset_t streamLength)
 {
-  const long offset = file->rfind("wvpk", streamLength);
+  const offset_t offset = file->rfind("wvpk", streamLength);
   if(offset == -1)
     return 0;
 
diff --git a/taglib/wavpack/wavpackproperties.h b/taglib/wavpack/wavpackproperties.h
index 0f5d1dcb..fb3540a4 100644
--- a/taglib/wavpack/wavpackproperties.h
+++ b/taglib/wavpack/wavpackproperties.h
@@ -58,13 +58,13 @@ namespace TagLib {
        * \deprecated This constructor will be dropped in favor of the one below
        * in a future version.
        */
-      Properties(const ByteVector &data, long streamLength, ReadStyle style = Average);
+      Properties(const ByteVector &data, offset_t streamLength, ReadStyle style = Average);
 
       /*!
        * Create an instance of WavPack::Properties.
        */
       // BIC: merge with the above constructor
-      Properties(File *file, long streamLength, ReadStyle style = Average);
+      Properties(File *file, offset_t streamLength, ReadStyle style = Average);
 
       /*!
        * Destroys this WavPack::Properties instance.
@@ -137,8 +137,8 @@ namespace TagLib {
       Properties(const Properties &);
       Properties &operator=(const Properties &);
 
-      void read(File *file, long streamLength);
-      unsigned int seekFinalIndex(File *file, long streamLength);
+      void read(File *file, offset_t streamLength);
+      unsigned int seekFinalIndex(File *file, offset_t streamLength);
 
       class PropertiesPrivate;
       PropertiesPrivate *d;
diff --git a/taglib/xm/xmfile.cpp b/taglib/xm/xmfile.cpp
index 9192e9bf..61cfc77c 100644
--- a/taglib/xm/xmfile.cpp
+++ b/taglib/xm/xmfile.cpp
@@ -587,7 +587,7 @@ void XM::File::read(bool)
     READ_ASSERT(count == std::min(instrumentHeaderSize, (unsigned long)instrument.size() + 4));
 
     unsigned long sampleHeaderSize = 0;
-    long offset = 0;
+    offset_t offset = 0;
     if(sampleCount > 0) {
       sumSampleCount += sampleCount;
       // wouldn't know which header size to assume otherwise:
EOF

test -f mp4v2_win32.patch ||
  cat >mp4v2_win32.patch <<"EOF"
diff -ruN mp4v2-2.0.0.orig/GNUmakefile.am mp4v2-2.0.0/GNUmakefile.am
--- mp4v2-2.0.0.orig/GNUmakefile.am	2012-05-21 00:11:55.000000000 +0200
+++ mp4v2-2.0.0/GNUmakefile.am	2014-04-14 07:22:35.904963506 +0200
@@ -170,6 +170,7 @@
 endif
 if ADD_PLATFORM_WIN32
     libmp4v2_la_SOURCES += \
+        libplatform/platform_win32.cpp         \
         libplatform/io/File_win32.cpp          \
         libplatform/io/FileSystem_win32.cpp    \
         libplatform/number/random_win32.cpp    \
diff -ruN mp4v2-2.0.0.orig/libplatform/platform_win32.cpp mp4v2-2.0.0/libplatform/platform_win32.cpp
--- mp4v2-2.0.0.orig/libplatform/platform_win32.cpp	1970-01-01 01:00:00.000000000 +0100
+++ mp4v2-2.0.0/libplatform/platform_win32.cpp	2014-04-14 07:21:11.432961343 +0200
@@ -0,0 +1,1091 @@
+///////////////////////////////////////////////////////////////////////////////
+//
+//  The contents of this file are subject to the Mozilla Public License
+//  Version 1.1 (the "License"); you may not use this file except in
+//  compliance with the License. You may obtain a copy of the License at
+//  http://www.mozilla.org/MPL/
+//
+//  Software distributed under the License is distributed on an "AS IS"
+//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
+//  License for the specific language governing rights and limitations
+//  under the License.
+// 
+//  The Original Code is MP4v2.
+// 
+//  The Initial Developer of the Original Code is David Byron.
+//  Portions created by David Byron are Copyright (C) 2010.
+//  All Rights Reserved.
+//
+//  Contributors:
+//      David Byron, dbyron@dbyron.com
+//
+///////////////////////////////////////////////////////////////////////////////
+
+#include "src/impl.h"
+#include "libplatform/impl.h" /* for platform_win32_impl.h which declares Utf8ToFilename */
+#include <algorithm> /* for replace */
+#include <windows.h>
+
+namespace mp4v2 {
+    using namespace impl;
+}
+
+/**
+ * Set this to 1 to compile in extra debugging
+ */
+#define EXTRA_DEBUG 0
+
+/**
+ * @def LOG_PRINTF
+ *
+ * call log.printf if EXTRA_DEBUG is defined to 1.  Do
+ * nothing otherwise
+ */
+#if EXTRA_DEBUG
+#define LOG_PRINTF(X) log.printf X
+#else
+#define LOG_PRINTF(X)
+#endif
+
+/**
+ * Section 2.13 "Special Characters and Noncharacters" of
+ * _The Unicode Standard, Version 5.0_
+ * (http://www.unicode.org/versions/Unicode5.0.0/bookmarks.html)
+ * defines "The Replacement Character" U+FFFD as the
+ * "general substitute character" that "can be substituted
+ * for any 'unknown' character in another encoding that can
+ * not be mapped in terms of known Unicode characters"
+ *
+ * See also section D.7 of 10646.
+ */
+#define REPLACEMENT_CHAR    0xFFFD
+
+namespace mp4v2 { namespace platform { namespace win32 {
+
+/**
+ * A structure to store the number of characters required to
+ * encode a particular UCS-4 character in UTF-8
+ */
+struct utf8_len_info
+{
+    /**
+     * This structure applies to a number >= @p range_min.
+     */
+    UINT32      range_min;
+
+    /**
+     * This structure applies to a number <= @p range_max.
+     */
+    UINT32      range_max;
+
+    /**
+     * The number of characters required to encode a number
+     * in [@p range_min,@p range_max] as UTF-8.
+     */
+    size_t      num_chars;
+};
+
+/**
+ * A structure to store the number of characters required to
+ * encode a particular UCS-4 character in UTF-8.  For now
+ * we're using wide characters (which according to
+ * http://msdn.microsoft.com/en-us/library/ms776414.aspx
+ * means UTF-16 since Windows 2000) so we're only using up
+ * to 4-byte UTF-8 sequences.  Parts of the range aren't
+ * valid (e.g. [U+D800,U+DFFF] but that's handled elsewhere.
+ */
+static struct utf8_len_info s_len_info[] =
+{
+    { 0x00000000, 0x0000007F, 1 },
+    { 0x00000080, 0x000007FF, 2 },
+    { 0x00000800, 0x0000FFFF, 3 },
+    { 0x00010000, 0x001FFFFF, 4 },
+    { 0x00200000, 0x03FFFFFF, 5 },
+    { 0x04000000, 0x7FFFFFFF, 6 }
+};
+
+/**
+ * Utf8ToFilename constructor
+ *
+ * @param utf8string a UTF-8 encoded string that does not
+ * begin with \\\?\\ nor \\\?\\UNC\\
+ *
+ * @see IsValidUTF16 to see whether the constructor
+ * succeeded
+ */
+Utf8ToFilename::Utf8ToFilename( const string &utf8string )
+    : _wideCharString( NULL )
+      , utf8( _utf8 )
+{
+    // See
+    // http://msdn.microsoft.com/en-us/library/aa365247%28v=vs.85%29.aspx
+    // for notes about path lengths, prefixes, etc.  The
+    // goal is to support the longest path possible.
+    // Relative paths are limited to 260 characters but
+    // absolute paths can be up to about 32767
+    // characters if properly prefixed.
+
+    // If utf8string is a relative path, convert it to
+    // UTF-16 and be done.
+    if (!IsAbsolute(utf8string))
+    {
+        _wideCharString = ConvertToUTF16(utf8string);
+        return;
+    }
+
+    // Since the prefix has backslashes, convert any forward
+    // slashes in utf8string to backslashes to keep Windows
+    // happy
+    const string *utf8ToUse = &utf8string;
+    string forwardSlash;
+
+    if (utf8string.find('/') != std::string::npos)
+    {
+        forwardSlash = utf8string;
+        std::replace(forwardSlash.begin(),forwardSlash.end(),'/','\\');
+        utf8ToUse = &forwardSlash;
+    }
+    ASSERT(utf8ToUse);
+    ASSERT((*utf8ToUse).length() > 0);
+
+    // utf8string is an absolute path.  It could be a
+    // UNC path (\\host\path).  The prefix is different
+    // for UNC paths than it is for non-UNC paths.
+    string prefixedPath;
+
+    if (IsUncPath(*utf8ToUse))
+    {
+        // utf8string begins with two backslashes, but
+        // with a prefix we only need one so we can't
+        // just prepend a prefix.
+        prefixedPath = "\\\\?\\UNC" + (*utf8ToUse).substr(1);
+    }
+    else
+    {
+        prefixedPath = "\\\\?\\" + *utf8ToUse;
+    }
+
+    // Transform prefixedPath to UTF-16 so it's
+    // appropriate for CreateFileW
+    _wideCharString = ConvertToUTF16(prefixedPath);
+}
+
+Utf8ToFilename::~Utf8ToFilename( )
+{
+    if( _wideCharString != NULL )
+    {
+        free(_wideCharString);
+        _wideCharString = NULL;
+    }
+}
+
+/**
+ * Convert a UTF-8 encoded string to a UTF-16 string
+ *
+ * @param utf8 the NUL-terminated UTF-8 string to decode
+ *
+ * @retval NULL error allocating memory for UTF-16 string
+ *
+ * @retval non-NULL NUL-terminated UTF-16 version of @p
+ * utf8.  Invalid portions of UTF-8 are represented by a
+ * replacement character U+FFFD.  The caller is
+ * responsible for freeing this memory.
+ */
+wchar_t *
+Utf8ToFilename::ConvertToUTF16 ( const string &utf8string )
+{
+    int         num_bytes;
+    size_t      num_chars;
+    wchar_t     *retval;
+
+    ASSERT(sizeof(wchar_t) == 2);
+
+    // Store the utf8 string in our member variable so it's
+    // available
+    _utf8 = utf8string;
+
+    // We need to find out how many characters we're dealing
+    // with so we know how much memory to allocate.  At the
+    // same time, it's possible that the string we've been
+    // given isn't valid UTF-8.  So, just use the length of
+    // the string we've been given as the number of
+    // characters to allocate.  The decoded string can't be
+    // longer than this, even taking into account surrogate
+    // pairs since they require 4 UTF-8 characters but only
+    // two UTF-16 character elements.
+    num_chars = utf8string.length();
+
+    LOG_PRINTF((MP4_LOG_VERBOSE4,"%s: entry point (%d character string)",
+                __FUNCTION__,num_chars));
+
+    /*
+    ** Allocate space for the decoded string.  Add one
+    ** for the NUL terminator.
+    */
+    num_bytes = (num_chars + 1) * sizeof(wchar_t);
+    retval = (wchar_t *)malloc(num_bytes);
+    if (!retval)
+    {
+        log.errorf("%s: error allocating memory for %d byte(s)",__FUNCTION__,num_bytes);
+        return NULL;
+    }
+
+    /*
+    ** ConvertToUTF16Buf zeroes out the memory so don't
+    ** do it here
+    */
+
+    // ConvertToUTF16Buf shouldn't fail if we allocated
+    // enough memory for the entire string.  Check
+    // anyway just to be safe.
+    if (!ConvertToUTF16Buf(utf8string.c_str(),retval,num_bytes))
+    {
+        // But ASSERT so we can find the problem and fix
+        // it.
+        ASSERT(0);
+        free(retval);
+        retval = NULL;
+        return NULL;
+    }
+
+    return retval;
+}
+
+/**
+ * Convert a UTF-8 encoded string to a UTF-16 string in
+ * a previously allocated buffer.
+ *
+ * @param utf8 the NUL-terminated UTF-8 string to decode
+ *
+ * @param utf16_buf the buffer in which to place the
+ * UTF-16 version of @p utf8.  If there's enough space
+ * to hold a NUL terminator, @p utf16_buf contains one.
+ * If not, @p utf16_buf is not NUL terminated.
+ *
+ * @param num_bytes the number of bytes that @p
+ * utf16_str points to
+ *
+ * @retval 0 error converting @p name to UTF-16,
+ * including when @p utf8 requires more space to encode
+ * in UTF-16 than indicated by @p num_bytes.  In that
+ * case, @p utf16_buf contains the UTF-16 encoding of as
+ * much of @p utf8 as possible.
+ *
+ * @retval 1 successfully converted @p name to @p UTF-16
+ * in @p utf16_buf. wide character (UTF-16) version of
+ * @p Invalid portions of UTF-8 are represented by a
+ * replacement character U+FFFD.
+ */
+int
+Utf8ToFilename::ConvertToUTF16Buf ( const char      *utf8,
+                                    wchar_t         *utf16_buf,
+                                    size_t          num_bytes )
+{
+    size_t      i;
+    const UINT8 *next_char;
+    size_t      num_chars;
+    size_t      num_utf16_chars;
+    size_t      num_input_bytes;
+    const UINT8 *p;
+    wchar_t     this_utf16[2];
+
+    ASSERT(utf8);
+    ASSERT(utf16_buf || (num_bytes == 0));
+    ASSERT(sizeof(wchar_t) == 2);
+
+    ASSERT(num_bytes % sizeof(wchar_t) == 0);
+
+    LOG_PRINTF((MP4_LOG_VERBOSE4,"%s: converting \"%s\"",__FUNCTION__,utf8));
+
+    num_chars = strlen(utf8);
+
+    // If the input is NUL-terminated (which it better
+    // be), the NUL-terminator is a valid input byte as
+    // well
+    num_input_bytes = num_chars + 1;
+
+    // Make sure the buffer we've been given is long
+    // enough.  We might need one UTF-16 character for
+    // every UTF-8 character.  And one more for the NUL
+    // terminator.
+    //
+    // Here, check that there's room for a NUL
+    // terminator in the output string.  This makes it
+    // safe to dereference p in the while loop below.
+    // It's probably enough to check num_bytes == 0 here
+    // but if we did that we'd have to change the error
+    // message after the while loop to be less specific.
+    // This way we give the caller more info about the
+    // input string.
+    if (num_bytes < sizeof(wchar_t))
+    {
+        log.errorf("%s: %u byte(s) is not enough to transform a %u byte UTF-8 string "
+                   "to NUL-terminated UTF-16",__FUNCTION__,num_bytes,num_input_bytes);
+        return 0;
+    }
+
+    ASSERT(num_bytes > 0);
+    ASSERT(utf16_buf);
+    memset(utf16_buf,0,num_bytes);
+
+    // The number of UTF-16 characters we've got space for
+    // in utf16_buf
+    num_utf16_chars = num_bytes / sizeof(wchar_t);
+
+    p = (const UINT8 *)utf8;
+    i = 0;
+    while (*p && (i < num_utf16_chars))
+    {
+        LOG_PRINTF((MP4_LOG_VERBOSE4,"%s: decoding first UTF-8 byte 0x%02X (UTF-16 "
+                    "character %d of at most %d)",__FUNCTION__,*p,(i + 1),
+                    num_utf16_chars));
+
+        memset(this_utf16,0,sizeof(this_utf16));
+
+        // This function decodes illegal bytes/sequences
+        // with a replacement character and returns the
+        // pointer to the next character to decode.  Pass
+        // NULL since we don't care about detecting invalid
+        // characters here.
+        next_char = Utf8DecodeChar(p,num_input_bytes,this_utf16,NULL);
+
+        // We've always got one character to assign
+        utf16_buf[i++] = this_utf16[0];
+
+        // If we're dealing with a surrogate pair,
+        // assign the low half too
+        if (this_utf16[1])
+        {
+            // We may not have any more room in the
+            // UTF-16 buffer.  Check to make sure we
+            // don't step on someone else's memory.  We
+            // need to return failure here instead of
+            // depending on our other logic to do it for
+            // us.  We'll get out of the while loop with
+            // no extra code, but if we're dealing with
+            // the UTF-16 encoding of the last character
+            // in the input string, there won't appear
+            // to be anything wrong.
+            if (i >= num_utf16_chars)
+            {
+                log.errorf("%s: out of space in %u  byte output string to store surrogate "
+                           "pair low half (0x%04X)",__FUNCTION__,num_bytes,this_utf16[1]);
+                return 0;
+            }
+             
+            utf16_buf[i++] = this_utf16[1];
+        }
+
+        // Put this here to make it brutally clear that
+        // the cast is safe
+        ASSERT(next_char >= p);
+        num_input_bytes -= (size_t)(next_char - p);
+        p = next_char;
+    }
+
+    if (*p)
+    {
+        // Since num_input_bytes includes 1 for the
+        // NUL-terminator, it's got to be bigger than
+        // one here.
+        ASSERT(num_input_bytes > 1);
+        log.errorf("%s: %u byte(s) of input string remain(s) undecoded (%s): out of space in "
+                   "%u byte output string",__FUNCTION__,(num_input_bytes - 1),p,num_bytes);
+        return 0;
+    }
+
+    return 1;
+}
+
+/**
+ * Accessor for the length of a prefix (i.e. \\\?\\ or
+ * \\\?\\UNC\\) that begins a filename
+ *
+ * @param utf8string the UTF-8 encoded filename to
+ * examine
+ *
+ * @return the length of the prefix of @p utf8string in
+ * characters
+ */
+int
+Utf8ToFilename::GetPrefixLen ( const string &utf8string )
+{
+    if (utf8string.find("\\\\?\\") == 0)
+    {
+        return strlen("\\\\?\\");
+    }
+
+    if (utf8string.find("\\\\?\\UNC\\") == 0)
+    {
+        return strlen("\\\\?\\UNC\\");
+    }
+
+    return 0;
+}
+
+/**
+ * Determine if a path is absolute or not
+ *
+ * @param utf8string the UTF-8 encoded path to examine
+ * that does not begin with \\\?\\ nor \\\?\\UNC\\
+ *
+ * @retval 0 @p utf8string is not an absolute path
+ * @retval 1 @p utf8string is an absolute path
+ */       
+int
+Utf8ToFilename::IsAbsolute ( const string &utf8string )
+{
+    // Assume utf8string doesn't already start with a
+    // long filename prefix (i.e. \\?\ or \\?\UNC\)
+    // since the logic here depends on that.
+    ASSERT(GetPrefixLen(utf8string) == 0);
+
+    // Is an empty string absolute or relative?  It's
+    // not absolute since we can't tell what
+    // drive/volume it's for so say it's relative.
+    if (utf8string.length() == 0)
+    {
+        return 0;
+    }
+        
+    // Here we're looking for:
+    //  x:   drive relative
+    //  x:\  absolute path
+    if (utf8string[1] == ':')
+    {
+        // It starts with x:, but is it x:/ ?
+        if ((utf8string.length() >= 2) && IsPathSeparator(utf8string[2]))
+        {
+            // Yup -- it's absolute
+            return 1;
+        }
+
+        // Nope, not x:/, just x:something
+        return 0;
+    }
+
+    // UNC paths are absolute paths too
+    return IsUncPath(utf8string);
+}
+
+/**
+ * Determine if a character is a valid path separator
+ *
+ * @param c the character to check
+ *
+ * @retval 0 @p c is not a valid path separator
+ * @retval 1 @p c is a valid path separator
+ */
+int
+Utf8ToFilename::IsPathSeparator ( char c )
+{
+    return ((c == '\\') || (c == '/'));
+}
+
+/**
+ * Determine if a path is a UNC path
+ *
+ * @param utf8string the UTF-8 encoded path to examine
+ * that does not begin with \\\?\\ nor \\\?\\UNC\\
+ *
+ * @retval 0 @p utf8string is not a UNC path
+ * @retval 1 @p utf8string is a UNC path
+ */       
+int
+Utf8ToFilename::IsUncPath ( const string &utf8string )
+{
+    const char  *host;
+    int         num_slashes;
+    const char  *p;
+
+    // Assume utf8string doesn't already start with a
+    // long filename prefix (i.e. \\?\ or \\?\UNC\)
+    // since the logic here depends on that.
+    ASSERT(GetPrefixLen(utf8string) == 0);
+
+    // Is an empty string a UNC path?  No.
+    if (utf8string.length() == 0)
+    {
+        return 0;
+    }
+
+    //  Recognize:
+    //    //volume/path
+    //    \\volume\path
+    if (!IsPathSeparator(utf8string[0]))
+    {
+        // If it doesn't start with a path separator, it's
+        // not a UNC path.
+        return 0;
+    }
+
+    // The path starts with a slash, so it could be a UNC
+    // path.  See if it starts with two slashes...Be careful
+    // though, it might have more than 2 slashes.
+    p = utf8string.c_str();
+    num_slashes = 0;
+    while (*p && IsPathSeparator(*p))
+    {
+        num_slashes++;
+        p++;
+    }
+
+    // We found a slash at the beginning so we better have
+    // at least one here
+    ASSERT(num_slashes >= 1);
+    if ((num_slashes > 2) || !(*p))
+    {
+        // If we've got more than two slashes or we've
+        // run off the end of the string (///foo or
+        // //)...who knows how the OS will handle it,
+        // but it's not a UNC path.
+        log.errorf("%s: don't understand path(%s)",__FUNCTION__,utf8string.c_str());
+        return 0;
+    }
+
+    // If we've only got one slash, it looks like a
+    // drive relative path.  If it's something like
+    // /foo//bar it's not clear how the OS handles it,
+    // but that's someone else's problem.  It's not a
+    // UNC path.
+    if (num_slashes == 1)
+    {
+        return 0;
+    }
+    
+    // If we're here, we've got two slashes followed by
+    // a non-slash.  Something like //foo.  To be a
+    // proper UNC path, we need to see a hostname
+    // (e.g. foo), and then another slash.  If not, it's
+    // not a UNC path.
+    ASSERT(num_slashes == 2);
+
+    // Tempting to use STRTOK_R here, but that modifies
+    // the original string.  Instead of making a copy,
+    // search manually.
+    host = p;
+    while (*p && !IsPathSeparator(*p))
+    {
+        p++;
+    }
+
+    // We checked for separators above, so we better
+    // have moved on at least a bit
+    ASSERT(host != p);
+    if (!(*p))
+    {
+        // We ran off the end of the string without finding
+        // another separator.  So, we've got something like
+        // 
+        //  //foobar
+        // 
+        // which isn't a UNC path.
+        log.warningf("%s: incomplete UNC path: host only(%s)",__FUNCTION__,
+                     utf8string.c_str());
+        return 0;
+    }
+
+    // p points to a separator, so...we've got one of:
+    //  //host//
+    //  //host//blah
+    //  //host/bar
+    //
+    // Of these, only the last is a proper UNC path.  See
+    // what we've got after p.
+    num_slashes = 0;
+    while (*p && IsPathSeparator(*p))
+    {
+        num_slashes++;
+        p++;
+    }
+
+    // We better have at least one slash or our logic is
+    // broken
+    ASSERT(num_slashes >= 1);
+    if (!(*p))
+    {
+        // //host// (or maybe //host///), but no path
+        // part after the host
+        log.warningf("%s: incomplete UNC path: no path after host(%s)",
+                     __FUNCTION__,utf8string.c_str());
+        return 0;
+    }
+
+    if (num_slashes > 1)
+    {
+        // Another busted case //host//blah or
+        // //host///blah, etc.
+        log.warningf("%s: invalid UNC path: too many slashes after host(%s)",
+                     __FUNCTION__,utf8string.c_str());
+        return 0;
+    }
+    
+    // If we're here it means num_slashes is exactly 1
+    // so we've got //host/something so we're calling
+    // that a UNC path.
+    return 1;
+}
+
+/**
+ * Accessor for whether the UTF-16 encoded string is valid
+ *
+ * @retval false the UTF-16 encoded string is not valid
+ * @retval true the UTF-16 encoded string is valid
+ */
+bool
+Utf8ToFilename::IsUTF16Valid( ) const
+{
+    return (_wideCharString ? true : false);
+}
+
+/**
+ * Decode one UTF-8 encoded character into a UTF-16
+ * character.  The trouble here is that UTF-16 is really a
+ * variable length encoding to handle surrogate pairs
+ * (0xD800 --> 0xDFFF).  This way UTF-16 can handle more
+ * than 2^16 characters.  So we need to be careful.  UCS-2
+ * is a fixed width (16-bit) encoding that we could use, but
+ * then we can only handle 2^16 characters (the BMP).  To
+ * handle all 2^21 characters, we need UTF-16.
+ *
+ * What does Windows really use?  UTF-16.  See
+ * http://unicode.org/iuc/iuc17/b2/slides.ppt for a
+ * discussion.
+ * http://discuss.fogcreek.com/joelonsoftware5/default.asp?cmd=show&ixPost=168543
+ * also has some info.
+ *
+ * @param utf8_char the UTF-8 character to decode, possibly
+ * occupying multiple bytes, not necessarily NUL terminated
+ *
+ * @param num_bytes the number of bytes that @p utf8_char
+ * points to (must be > 0)
+ *
+ * @param utf16 populated with the UTF-16 equivalent of @p
+ * utf8_char.  Note that this must point to at least 2
+ * wchar_t's of memory so there's room to hold a surrogate
+ * pair.
+ *
+ * @param invalid populated with 1 if @p utf8_char doesn't
+ * point to a valid UTF-8 encoded character, 0 if @p
+ * utf8_char is valid.
+ *
+ * @return the next byte to examine for subsequent decoding
+ * (some number of bytes after @p utf8_char).  This may not
+ * be valid to dereference depending on the value of @p
+ * num_bytes.
+ */
+const UINT8 *
+Utf8ToFilename::Utf8DecodeChar ( const UINT8    *utf8_char,
+                                 size_t         num_bytes,
+                                 wchar_t        *utf16,
+                                 int            *invalid )
+
+{
+    wchar_t     high_half;
+    int         i;
+    UINT8       len;
+    wchar_t     low_half;
+    UINT8       mask;
+    const UINT8 *p;
+    UINT32      ucs4;
+    int         valid_len;
+
+    ASSERT(utf8_char);
+    ASSERT(num_bytes > 0);
+    ASSERT(utf16);
+
+    LOG_PRINTF((MP4_LOG_VERBOSE4,"%s: decoding UTF-8 string at address 0x%p",
+                __FUNCTION__,utf8_char));
+
+    /*
+    ** Assume utf8_char is invalid until we learn otherwise
+    */
+    if (invalid)
+    {
+        *invalid = 1;
+    }
+
+    /*
+    ** Traverse the UTF-8 encoding and figure out what we've
+    ** got.
+    */
+    p = (const UINT8 *)(utf8_char);
+
+    /*
+    ** This is the number of bytes we expect based on the
+    ** first octet.  If subsequent bytes are NUL or invalid,
+    ** then it may not the same as the actual len.
+    */
+    len = Utf8NumOctets(*p);
+    if (len == 0)
+    {
+        log.errorf("%s: 0x%02X is not a valid first byte of a UTF-8 encoded character",__FUNCTION__,*p);
+
+        /*
+        ** Use the replacement character and advance past
+        ** the invalid byte
+        */
+        *utf16 = REPLACEMENT_CHAR;
+        return p + 1;
+    }
+
+    /*
+    ** Handle one byte encodings in a special case.  See
+    ** below for an explanation of how we mask successive
+    ** bytes of an encoding to see why.  We're depending on
+    ** the validation in Utf8NumOctets here to make this OK.
+    */
+    if (len == 1)
+    {
+        /*
+        ** There's no intermediate UCS-4 step here.  We go
+        ** straight to UTF-16 since they're the same.
+        */
+        LOG_PRINTF((MP4_LOG_VERBOSE4,"%s: one byte UTF-16 encoding: 0x%02X",
+                    __FUNCTION__,*p));
+        *utf16 = *p;
+        if (invalid)
+        {
+            *invalid = 0;
+        }
+        return p + 1;
+    }
+
+    /*
+    ** Make sure we've got enough bytes in our input string
+    ** to form a valid UTF-8 character
+    */
+    if (len > num_bytes)
+    {
+        log.errorf("%s: first byte 0x%02X indicates a %d byte "
+                   "UTF-8 character, but we only have %u valid byte(s)",
+                   __FUNCTION__,*p,len,num_bytes);
+        *utf16 = REPLACEMENT_CHAR;
+        return p + 1;
+    }
+
+    /*
+    ** Traverse the bytes that should be part of this UTF-8
+    ** encoded character and make sure we don't have an
+    ** overlength encoding, and make sure that each
+    ** character is valid.
+    */
+
+    /*
+    ** As we traverse each character, we mask off the
+    ** appropriate number of bits and include them in the
+    ** overall result.
+    **
+    ** 1 byte encoding [U+00000000,U+0000007F]: 7 bits (7 bits total) (handled above)
+    ** 2 byte encoding [U+00000080,U+000007FF]: 5 bits, 6 bits (11 bits total)
+    ** 3 byte encoding [U+00000800,U+0000FFFF]: 4 bits, 6 bits, 6 bits (16 bits total)
+    ** 4 byte encoding [U+00010000,U+001FFFFF]: 3 bits, 6 bits, 6 bits, 6 bits (21 bits total)
+    ** 5 byte encoding [U+00200000,U+03FFFFFF]: 2 bits, 6 bits, 6 bits, 6 bits, 6 bits (26 bits total)
+    ** 6 byte encoding [U+04000000,U+7FFFFFFF]: 1 bit, 6 bits, 6 bits, 6 bits, 6 bits, 6 bits (31 bits total)
+    **
+    ** So, mask the initial byte appropriately, then take
+    ** the bottom 6 bits from the remaining bytes.  To be
+    ** brutally explicit, the first byte mask is:
+    **
+    ** 1 byte encoding: 0x7F (or 0x80 - 1) (or (1 << 7) - 1)
+    ** 2 byte encoding: 0x1F (or 0x20 - 1) (or (1 << 5) - 1)
+    ** 3 byte encoding: 0x0F (or 0x10 - 1) (or (1 << 4) - 1)
+    ** 4 byte encoding: 0x07 (or 0x08 - 1) (or (1 << 3) - 1)
+    ** 5 byte encoding: 0x03 (or 0x04 - 1) (or (1 << 2) - 1)
+    ** 6 byte encoding: 0x01 (or 0x02 - 1) (or (1 << 1) - 1)
+    **    
+    ** So, the one byte encoding is a special case (again,
+    ** handled above), but for the other lengths, the mask
+    ** is (1 << (7 - len)) - 1.
+    */
+
+    /*
+    ** Handle the first byte of multi-byte encodings since
+    ** it's special
+    */
+    ASSERT(len > 1);
+    ASSERT(len <= 6);
+    mask = (1 << (7 - len)) - 1;
+    ucs4 = *p & mask;
+    p++;
+
+    /*
+    ** Now handle the remaining bytes
+    */
+    for (i = 1;(i < len);i++)
+    {
+        if ((*p < 0x80) || (*p > 0xBF))
+        {
+            log.errorf("%s: 0x%02X is not a valid continuation character in a UTF-8 encoding",
+                       __FUNCTION__,*p);
+
+            /*
+            ** Use the replacement character and return the
+            ** next byte after the invalid sequence as the
+            ** place for subsequent decoding operations.  In
+            ** this case the invalid continuation character
+            ** could be the beginning of the next valid
+            ** sequence, so return that.
+            */
+            *utf16 = REPLACEMENT_CHAR;
+            return p;
+        }
+            
+        /*
+        ** For the remainder of the bytes, shift over what
+        ** we've already got by 6 bits, and then OR in the
+        ** bottom 6 bits of the current byte.
+        */
+        ucs4 = (ucs4 << 6) | (*p & 0x3F);
+        p++;
+    }
+
+    /*
+    ** p is now pointing to the beginning of the next UTF-8
+    ** sequence to decode...
+    */
+
+    /*
+    ** Finally, detect overlong encodings.  For example, a
+    ** line feed (U+000A) should be encoded as 0x0A
+    ** (0b00001010) but could in theory be encoded in UTF-8
+    ** as 0xC0 0x8A (0b10001010).
+    **
+    ** Another example is the forward slash (/) (U+002F).
+    ** It should be encoded as 0x2F, but could in theory be
+    ** encoded in UTF-8 as 0xC0 0xAF (which we'll catch
+    ** because 0xC0 is an invalid first byte of a UTF-8
+    ** encoding), but could also be 0xE0 0x80 0xAF.
+    **
+    ** I can't see any reasonable way to do this other than
+    ** to check the decoded character against its expected
+    ** length
+    */
+    valid_len = Utf8LenFromUcs4(ucs4);
+    if (valid_len == 0)
+    {
+        /*
+        ** This should never happen
+        */
+        log.errorf("%s: decoded a character that we can't encode again (0x%08X)",__FUNCTION__,ucs4);
+        ASSERT(0);
+
+        /*
+        ** If it does, use the replacement character
+        */
+        *utf16 = REPLACEMENT_CHAR;
+        return p;
+    }
+
+    if (len != valid_len)
+    {
+        ASSERT(len > valid_len);
+        log.errorf("%s: overlong encoding(%s)...should be %d byte(s), not %d",__FUNCTION__,
+                   utf8_char,valid_len,len);
+        *utf16 = REPLACEMENT_CHAR;
+        return p;
+    }
+
+    /*
+    ** UTF-16 can only hold 21 bits.  As of now (21-dec-10),
+    ** there's no Unicode code point bigger than 2^21.  To
+    ** be safe, check...
+    */
+    if (ucs4 > 0x0010FFFF)
+    {
+        log.errorf("%s: code point 0x%08X is too big",__FUNCTION__,ucs4);
+        *utf16 = REPLACEMENT_CHAR;
+        return p;
+    }
+
+    /*
+    ** Check to make sure we're not working with a "code
+    ** point" that is in the range used to indicate
+    ** surrogate pairs.
+    */
+    if ((ucs4 >= 0x0000D800) && (ucs4 <= 0x0000DFFF))
+    {
+        log.errorf("%s: code point 0x%08X is in the range used to indicate surrogate pairs",
+                   __FUNCTION__,ucs4);
+        *utf16 = REPLACEMENT_CHAR;
+        return p;
+    }
+
+    /*
+    ** To (try to) be complete, check for a couple more
+    ** invalid code points
+    */
+    if ((ucs4 == 0x0000FFFF) || (ucs4 == 0x0000FFFE))
+    {
+        log.errorf("%s: invalid code point (0x%08X)",__FUNCTION__,ucs4);
+        *utf16 = REPLACEMENT_CHAR;
+        return p;
+    }
+
+    /*
+    ** Finally, convert from UCS-4 to UTF-16.  This may be a
+    ** straightforward assignment, but we have to deal with
+    ** surrogate pairs
+    */
+    if (ucs4 <= 0x0000FFFF)
+    {
+        *utf16 = ucs4 & 0xFFFF;
+        LOG_PRINTF((MP4_LOG_VERBOSE4,"%s: UTF-16 encoding of 0x%08X is 0x%04X",
+                    __FUNCTION__,ucs4,*utf16));
+        if (invalid)
+        {
+            *invalid = 0;
+        }
+        return p;
+    }
+
+    /*
+    ** Transform UCS-4 into a UTF-16 surrogate pair
+    */
+
+    /*
+    ** Grab bits [10,20] (where bit 0 is the LSB) and shift
+    ** them down
+    */
+    high_half = 0xD800 + ((ucs4 - 0x00010000) >> 10);
+
+    /*
+    ** And the bottom 10 bits [0,9]
+    */
+    low_half = 0xDC00 + (ucs4 & 0x03FF);
+
+    utf16[0] = high_half;
+    utf16[1] = low_half;
+
+    LOG_PRINTF((MP4_LOG_VERBOSE4,"%s: UTF-16 encoding of 0x%08X is 0x%04X:0x%04X",
+                __FUNCTION__,ucs4,utf16[0],utf16[1]));
+
+    if (invalid)
+    {
+        *invalid = 0;
+    }
+
+    return p;
+}
+
+/**
+ * Determine the number of bytes required to hold the UTF-8
+ * encoding of a UCS-4 code point
+ *
+ * @param ucs4 the code point
+ *
+ * @param use_syslog 1 to use syslog, 0 otherwise
+ *
+ * @retval 0 @p ucs4 is not a valid code point
+ *
+ * @retval [1,6] the number of bytes required to hold the
+ * UTF-8 encoding of @p ucs4
+ */
+size_t
+Utf8ToFilename::Utf8LenFromUcs4 ( UINT32 ucs4 )
+{
+    size_t      table_idx;
+
+    LOG_PRINTF((MP4_LOG_VERBOSE4,"%s: processing UCS-4 code point 0x%08X",
+                __FUNCTION__,ucs4));
+
+    for (table_idx = 0;(table_idx < (sizeof(s_len_info) /
+                                     sizeof(struct utf8_len_info)));
+         table_idx++)
+    {
+        if ((s_len_info[table_idx].range_min <= ucs4) &&
+            (ucs4 <= s_len_info[table_idx].range_max))
+        {
+            return s_len_info[table_idx].num_chars;
+        }
+    }
+
+    log.errorf("%s: 0x%08X is an invalid code point",__FUNCTION__,ucs4);
+
+    return 0;
+}
+
+/**
+ * Determine the number of octets that a UTF-8 encoded
+ * character should occupy based on its first byte
+ *
+ * @param utf8_first_byte the byte to examine
+ *
+ * @retval 0 @p utf8_first_byte is not a valid first byte of
+ * a UTF-8 encoded character
+ *
+ * @retval [1,6] the number of octets that @p
+ * utf8_first_byte should occupy
+ */
+UINT8
+Utf8ToFilename::Utf8NumOctets ( UINT8 utf8_first_byte )
+{
+    /**
+     * Here's a mapping from the first byte of a UTF-8
+     * character to the number of bytes it should contain
+     * based on information from
+     * http://www.unicode.org/versions/corrigendum1.html as
+     * well as
+     * http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
+     *
+     * [0x00,0x7F]: 1       (0-127)     (128 possible values)
+     * [0x80,0xBF]: invalid (128-191)   (64 possible values)
+     * [0xC0,0xDF]: 2       (192-223)   (32 possible values) (see below)
+     * [0xE0,0xEF]: 3       (224-239)   (16 possible values)
+     * [0xF0,0xF7]: 4       (240 - 247) (8 possible values)
+     * [0xF8,0xFB]: 5       (248 - 251) (4 possible values)
+     * [0xFC,0xFD]: 6       (252 - 253) (2 possible values)
+     * [0xFE,0xFF]: invalid (254 - 255) (2 possible values)
+     *
+     * There's some gray area about 0xC0 and 0xC1.  It's
+     * clear they are invalid first bytes but the question
+     * is how to handle it.  If I reject them here, they'll
+     * get replaced with the REPLACEMENT character.  But, if
+     * I allow them here, it's likely that both this byte
+     * and the subsequent one will get replaced with only
+     * one replacement character.  This is what
+     * http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
+     * assumes in sections 4.1.1, 4.2.1 and 4.3.1.
+     */
+    if (utf8_first_byte <= 0x7F)
+    {
+        return 1;
+    }
+
+    if ((utf8_first_byte >= 0x80) && (utf8_first_byte <= 0xBF))
+    {
+        return 0;
+    }
+
+    if ((utf8_first_byte >= 0xC0) && (utf8_first_byte <= 0xDF))
+    {
+        return 2;
+    }
+
+    if ((utf8_first_byte >= 0xE0) && (utf8_first_byte <= 0xEF))
+    {
+        return 3;
+    }
+
+    if ((utf8_first_byte >= 0xF0) && (utf8_first_byte <= 0xF7))
+    {
+        return 4;
+    }
+
+    if ((utf8_first_byte >= 0xF8) && (utf8_first_byte <= 0xFB))
+    {
+        return 5;
+    }
+
+    if ((utf8_first_byte >= 0xFC) && (utf8_first_byte <= 0xFD))
+    {
+        return 6;
+    }
+
+    ASSERT((utf8_first_byte == 0xFE) || (utf8_first_byte == 0xFF));
+    return 0;
+}
+
+///////////////////////////////////////////////////////////////////////////////
+
+}}} // namespace mp4v2::platform::win32
diff -ruN mp4v2-2.0.0.orig/libplatform/platform_win32.h mp4v2-2.0.0/libplatform/platform_win32.h
--- mp4v2-2.0.0.orig/libplatform/platform_win32.h	2012-05-21 00:11:55.000000000 +0200
+++ mp4v2-2.0.0/libplatform/platform_win32.h	2014-04-14 07:53:02.677010281 +0200
@@ -7,6 +7,8 @@
 #ifdef __MINGW32__
 #   undef  __MSVCRT_VERSION__
 #   define __MSVCRT_VERSION__ 0x800
+// JAN: see http://code.google.com/p/mp4v2/issues/detail?id=132
+#   define _USE_32BIT_TIME_T
 #endif
 
 // set minimum win32 API requirement to Windows 2000 or higher
diff -ruN mp4v2-2.0.0.orig/libplatform/platform_win32_impl.h mp4v2-2.0.0/libplatform/platform_win32_impl.h
--- mp4v2-2.0.0.orig/libplatform/platform_win32_impl.h	1970-01-01 01:00:00.000000000 +0100
+++ mp4v2-2.0.0/libplatform/platform_win32_impl.h	2014-04-14 07:54:09.737011998 +0200
@@ -0,0 +1,70 @@
+// Note that we have a separate platform_win32_impl.h to deal with the fact that windows.h defines a macro
+// called FindAtom, which mp4v2 also defines.  In older versions of visual studio, this actually causes
+// some pretty seriously issues with naming collisions and the defined macros (think infamous min/max macro
+// of windows.h vs stdc++'s min/max template functions)
+#include <windows.h>
+
+typedef unsigned char UINT8;
+
+///////////////////////////////////////////////////////////////////////////////
+
+namespace mp4v2 { namespace platform { namespace win32 {
+
+class Utf8ToFilename
+{
+    public:
+    Utf8ToFilename( const string &utf8string );
+    ~Utf8ToFilename( );
+
+    bool                IsUTF16Valid( ) const;
+
+    operator LPCWSTR( ) const { return _wideCharString; }
+    operator LPWSTR( ) const { return _wideCharString; }
+
+    private:
+    Utf8ToFilename ( const Utf8ToFilename &src );
+    Utf8ToFilename &operator= ( const Utf8ToFilename &src );
+    
+    wchar_t             *ConvertToUTF16 ( const string &utf8 );
+
+    static int          ConvertToUTF16Buf ( const char      *utf8,
+                                            wchar_t         *utf16_buf,
+                                            size_t          num_bytes );
+    static int          GetPrefixLen ( const string &utf8string );
+
+    static int          IsAbsolute ( const string &utf8string );
+
+    static int          IsPathSeparator ( char c );
+
+    static int          IsUncPath ( const string &utf8string );
+
+    static const UINT8  *Utf8DecodeChar (
+        const UINT8     *utf8_char,
+        size_t          num_bytes,
+        wchar_t         *utf16,
+        int             *invalid
+        );
+
+    static size_t       Utf8LenFromUcs4 ( UINT32 ucs4 );
+
+    static UINT8        Utf8NumOctets ( UINT8 utf8_first_byte );
+
+    /**
+     * The UTF-8 encoding of the filename actually used
+     */
+    string      _utf8;
+
+    /**
+     * The UTF-16 encoding of the filename actually used
+     */
+    wchar_t*    _wideCharString;
+
+    public:
+
+    /**
+     * Accessor for @p _utf8
+     */
+    const string&       utf8;
+};
+
+}}} // namespace mp4v2::platform::win32
EOF

test -f mp4v2_clang6.patch ||
  cat >mp4v2_clang6.patch <<"EOF"
diff -ru mp4v2-2.0.0.orig/libutil/Utility.cpp mp4v2-2.0.0/libutil/Utility.cpp
--- mp4v2-2.0.0.orig/libutil/Utility.cpp	2012-05-21 00:11:53.000000000 +0200
+++ mp4v2-2.0.0/libutil/Utility.cpp	2018-12-07 06:52:54.298395112 +0100
@@ -531,26 +531,26 @@
                 printHelp( false, false );
                 return SUCCESS;
 
-            case LC_DEBUG:
+            case (int)LC_DEBUG:
                 debugUpdate( std::strtoul( prog::optarg, NULL, 0 ) );
                 break;
 
-            case LC_VERBOSE:
+            case (int)LC_VERBOSE:
             {
                 const uint32_t level = std::strtoul( prog::optarg, NULL, 0 );
                 _verbosity = ( level < 4 ) ? level : 3;
                 break;
             }
 
-            case LC_HELP:
+            case (int)LC_HELP:
                 printHelp( true, false );
                 return SUCCESS;
 
-            case LC_VERSION:
+            case (int)LC_VERSION:
                 printVersion( false );
                 return SUCCESS;
 
-            case LC_VERSIONX:
+            case (int)LC_VERSIONX:
                 printVersion( true );
                 return SUCCESS;
 
diff -ru mp4v2-2.0.0.orig/src/mp4.cpp mp4v2-2.0.0/src/mp4.cpp
--- mp4v2-2.0.0.orig/src/mp4.cpp	2012-05-21 00:11:53.000000000 +0200
+++ mp4v2-2.0.0/src/mp4.cpp	2018-12-07 06:43:25.145879532 +0100
@@ -870,7 +870,7 @@
         }
 
         catch (...) {
-            return MP4_INVALID_TRACK_ID;
+            return (mp4v2_ismacrypParams *)MP4_INVALID_TRACK_ID;
         }
     }
 
diff -ru mp4v2-2.0.0.orig/src/mp4util.h mp4v2-2.0.0/src/mp4util.h
--- mp4v2-2.0.0.orig/src/mp4util.h	2012-05-21 00:11:53.000000000 +0200
+++ mp4v2-2.0.0/src/mp4util.h	2018-12-07 06:38:56.557204609 +0100
@@ -33,7 +33,7 @@
 #ifndef ASSERT
 #   define ASSERT(expr) \
         if (!(expr)) { \
-            throw new Exception("assert failure: "LIBMPV42_STRINGIFY((expr)), __FILE__, __LINE__, __FUNCTION__ ); \
+            throw new Exception("assert failure: " LIBMPV42_STRINGIFY((expr)), __FILE__, __LINE__, __FUNCTION__ ); \
         }
 #endif
 
diff -ru mp4v2-2.0.0.orig/util/mp4art.cpp mp4v2-2.0.0/util/mp4art.cpp
--- mp4v2-2.0.0.orig/util/mp4art.cpp	2012-05-21 00:11:55.000000000 +0200
+++ mp4v2-2.0.0/util/mp4art.cpp	2018-12-07 06:54:49.929641777 +0100
@@ -377,11 +377,11 @@
     handled = true;
 
     switch( code ) {
-        case LC_ART_ANY:
+        case (int)LC_ART_ANY:
             _artFilter = numeric_limits<uint32_t>::max();
             break;
 
-        case LC_ART_INDEX:
+        case (int)LC_ART_INDEX:
         {
             istringstream iss( prog::optarg );
             iss >> _artFilter;
@@ -390,29 +390,29 @@
             break;
         }
 
-        case LC_LIST:
+        case (int)LC_LIST:
             _action = &ArtUtility::actionList;
             break;
 
-        case LC_ADD:
+        case (int)LC_ADD:
             _action = &ArtUtility::actionAdd;
             _artImageFile = prog::optarg;
             if( _artImageFile.empty() )
                 return herrf( "invalid image file: empty-string\n" );
             break;
 
-        case LC_REMOVE:
+        case (int)LC_REMOVE:
             _action = &ArtUtility::actionRemove;
             break;
 
-        case LC_REPLACE:
+        case (int)LC_REPLACE:
             _action = &ArtUtility::actionReplace;
             _artImageFile = prog::optarg;
             if( _artImageFile.empty() )
                 return herrf( "invalid image file: empty-string\n" );
             break;
 
-        case LC_EXTRACT:
+        case (int)LC_EXTRACT:
             _action = &ArtUtility::actionExtract;
             break;
 
diff -ru mp4v2-2.0.0.orig/util/mp4chaps.cpp mp4v2-2.0.0/util/mp4chaps.cpp
--- mp4v2-2.0.0.orig/util/mp4chaps.cpp	2012-05-21 00:11:55.000000000 +0200
+++ mp4v2-2.0.0/util/mp4chaps.cpp	2018-12-07 06:56:37.808767340 +0100
@@ -634,32 +634,32 @@
 
     switch( code ) {
         case 'A':
-        case LC_CHPT_ANY:
+        case (int)LC_CHPT_ANY:
             _ChapterType = MP4ChapterTypeAny;
             break;
 
         case 'Q':
-        case LC_CHPT_QT:
+        case (int)LC_CHPT_QT:
             _ChapterType = MP4ChapterTypeQt;
             break;
 
         case 'N':
-        case LC_CHPT_NERO:
+        case (int)LC_CHPT_NERO:
             _ChapterType = MP4ChapterTypeNero;
             break;
 
         case 'C':
-        case LC_CHPT_COMMON:
+        case (int)LC_CHPT_COMMON:
             _ChapterFormat = CHPT_FMT_COMMON;
             break;
 
         case 'l':
-        case LC_CHP_LIST:
+        case (int)LC_CHP_LIST:
             _action = &ChapterUtility::actionList;
             break;
 
         case 'e':
-        case LC_CHP_EVERY:
+        case (int)LC_CHP_EVERY:
         {
             istringstream iss( prog::optarg );
             iss >> _ChaptersEvery;
@@ -675,7 +675,7 @@
             _action = &ChapterUtility::actionExport;
             break;
 
-        case LC_CHP_EXPORT:
+        case (int)LC_CHP_EXPORT:
             _action = &ChapterUtility::actionExport;
             /* currently not supported since the chapters of n input files would be written to one chapter file
             _ChapterFile = prog::optarg;
@@ -690,7 +690,7 @@
             _action = &ChapterUtility::actionImport;
             break;
 
-        case LC_CHP_IMPORT:
+        case (int)LC_CHP_IMPORT:
             _action = &ChapterUtility::actionImport;
             /* currently not supported since the chapters of n input files would be read from one chapter file
             _ChapterFile = prog::optarg;
@@ -702,12 +702,12 @@
             break;
 
         case 'c':
-        case LC_CHP_CONVERT:
+        case (int)LC_CHP_CONVERT:
             _action = &ChapterUtility::actionConvert;
             break;
 
         case 'r':
-        case LC_CHP_REMOVE:
+        case (int)LC_CHP_REMOVE:
             _action = &ChapterUtility::actionRemove;
             break;
 
diff -ru mp4v2-2.0.0.orig/util/mp4file.cpp mp4v2-2.0.0/util/mp4file.cpp
--- mp4v2-2.0.0.orig/util/mp4file.cpp	2012-05-21 00:11:55.000000000 +0200
+++ mp4v2-2.0.0/util/mp4file.cpp	2018-12-07 06:57:29.218284178 +0100
@@ -190,15 +190,15 @@
     handled = true;
 
     switch( code ) {
-        case LC_LIST:
+        case (int)LC_LIST:
             _action = &FileUtility::actionList;
             break;
 
-        case LC_OPTIMIZE:
+        case (int)LC_OPTIMIZE:
             _action = &FileUtility::actionOptimize;
             break;
 
-        case LC_DUMP:
+        case (int)LC_DUMP:
             _action = &FileUtility::actionDump;
             break;
 
diff -ru mp4v2-2.0.0.orig/util/mp4subtitle.cpp mp4v2-2.0.0/util/mp4subtitle.cpp
--- mp4v2-2.0.0.orig/util/mp4subtitle.cpp	2012-05-21 00:11:55.000000000 +0200
+++ mp4v2-2.0.0/util/mp4subtitle.cpp	2018-12-07 06:58:11.247535746 +0100
@@ -165,25 +165,25 @@
     handled = true;
 
     switch( code ) {
-        case LC_LIST:
+        case (int)LC_LIST:
             _action = &SubtitleUtility::actionList;
             break;
 
-        case LC_EXPORT:
+        case (int)LC_EXPORT:
             _action = &SubtitleUtility::actionExport;
             _stTextFile = prog::optarg;
             if( _stTextFile.empty() )
                 return herrf( "invalid TXT file: empty-string\n" );
             break;
 
-        case LC_IMPORT:
+        case (int)LC_IMPORT:
             _action = &SubtitleUtility::actionImport;
             _stTextFile = prog::optarg;
             if( _stTextFile.empty() )
                 return herrf( "invalid TXT file: empty-string\n" );
             break;
 
-        case LC_REMOVE:
+        case (int)LC_REMOVE:
             _action = &SubtitleUtility::actionRemove;
             break;
 
diff -ru mp4v2-2.0.0.orig/util/mp4track.cpp mp4v2-2.0.0/util/mp4track.cpp
--- mp4v2-2.0.0.orig/util/mp4track.cpp	2012-05-21 00:11:55.000000000 +0200
+++ mp4v2-2.0.0/util/mp4track.cpp	2018-12-07 07:02:07.978741963 +0100
@@ -789,11 +789,11 @@
     handled = true;
 
     switch( code ) {
-        case LC_TRACK_WILDCARD:
+        case (int)LC_TRACK_WILDCARD:
             _trackMode = TM_WILDCARD;
             break;
 
-        case LC_TRACK_INDEX:
+        case (int)LC_TRACK_INDEX:
         {
             _trackMode = TM_INDEX;
             istringstream iss( prog::optarg );
@@ -803,7 +803,7 @@
             break;
         }
 
-        case LC_TRACK_ID:
+        case (int)LC_TRACK_ID:
         {
             _trackMode = TM_ID;
             istringstream iss( prog::optarg );
@@ -813,142 +813,142 @@
             break;
         }
 
-        case LC_LIST:
+        case (int)LC_LIST:
             _action = &TrackUtility::actionList;
             break;
 
-        case LC_COLR_PARMS:
+        case (int)LC_COLR_PARMS:
             _colorParameterItem.convertFromCSV( prog::optarg );
             break;
 
-        case LC_COLR_PARM_HD:
+        case (int)LC_COLR_PARM_HD:
             _colorParameterItem.primariesIndex        = 1;
             _colorParameterItem.transferFunctionIndex = 1;
             _colorParameterItem.matrixIndex           = 1;
             break;
 
-        case LC_COLR_PARM_SD:
+        case (int)LC_COLR_PARM_SD:
             _colorParameterItem.primariesIndex        = 6;
             _colorParameterItem.transferFunctionIndex = 1;
             _colorParameterItem.matrixIndex           = 6;
             break;
 
-        case LC_COLR_LIST:
+        case (int)LC_COLR_LIST:
             _action = &TrackUtility::actionColorParameterList;
             break;
 
-        case LC_ENABLED:
+        case (int)LC_ENABLED:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setEnabled;
             _actionTrackModifierSet_name     = "enabled";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_INMOVIE:
+        case (int)LC_INMOVIE:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setInMovie;
             _actionTrackModifierSet_name     = "inMovie";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_INPREVIEW:
+        case (int)LC_INPREVIEW:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setInPreview;
             _actionTrackModifierSet_name     = "inPreview";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_LAYER:
+        case (int)LC_LAYER:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setLayer;
             _actionTrackModifierSet_name     = "layer";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_ALTGROUP:
+        case (int)LC_ALTGROUP:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setAlternateGroup;
             _actionTrackModifierSet_name     = "alternateGroup";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_VOLUME:
+        case (int)LC_VOLUME:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setVolume;
             _actionTrackModifierSet_name     = "volume";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_WIDTH:
+        case (int)LC_WIDTH:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setWidth;
             _actionTrackModifierSet_name     = "width";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_HEIGHT:
+        case (int)LC_HEIGHT:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setHeight;
             _actionTrackModifierSet_name     = "height";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_LANGUAGE:
+        case (int)LC_LANGUAGE:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setLanguage;
             _actionTrackModifierSet_name     = "language";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_HDLRNAME:
+        case (int)LC_HDLRNAME:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setHandlerName;
             _actionTrackModifierSet_name     = "handlerName";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_UDTANAME:
+        case (int)LC_UDTANAME:
             _action = &TrackUtility::actionTrackModifierSet;
             _actionTrackModifierSet_function = &TrackModifier::setUserDataName;
             _actionTrackModifierSet_name     = "userDataName";
             _actionTrackModifierSet_value    = prog::optarg;
             break;
 
-        case LC_UDTANAME_R:
+        case (int)LC_UDTANAME_R:
             _action = &TrackUtility::actionTrackModifierRemove;
             _actionTrackModifierRemove_function = &TrackModifier::removeUserDataName;
             _actionTrackModifierRemove_name     = "userDataName";
             break;
 
-        case LC_COLR_ADD:
+        case (int)LC_COLR_ADD:
             _action = &TrackUtility::actionColorParameterAdd;
             break;
 
-        case LC_COLR_SET:
+        case (int)LC_COLR_SET:
             _action = &TrackUtility::actionColorParameterSet;
             break;
 
-        case LC_COLR_REMOVE:
+        case (int)LC_COLR_REMOVE:
             _action = &TrackUtility::actionColorParameterRemove;
             break;
 
-        case LC_PASP_PARMS:
+        case (int)LC_PASP_PARMS:
             _pictureAspectRatioItem.convertFromCSV( prog::optarg );
             break;
 
-        case LC_PASP_LIST:
+        case (int)LC_PASP_LIST:
             _action = &TrackUtility::actionPictureAspectRatioList;
             break;
 
-        case LC_PASP_ADD:
+        case (int)LC_PASP_ADD:
             _action = &TrackUtility::actionPictureAspectRatioAdd;
             break;
 
-        case LC_PASP_SET:
+        case (int)LC_PASP_SET:
             _action = &TrackUtility::actionPictureAspectRatioSet;
             break;
 
-        case LC_PASP_REMOVE:
+        case (int)LC_PASP_REMOVE:
             _action = &TrackUtility::actionPictureAspectRatioRemove;
             break;
 
--- mp4v2-2.0.0.orig/src/rtphint.cpp	2019-03-19 12:39:04.147742509 +0100
+++ mp4v2-2.0.0/src/rtphint.cpp	2019-03-19 12:38:42.239637221 +0100
@@ -339,7 +339,7 @@
                 pSlash = strchr(pSlash, '/');
                 if (pSlash != NULL) {
                     pSlash++;
-                    if (pSlash != '\0') {
+                    if (*pSlash != '\0') {
                         length = (uint32_t)strlen(pRtpMap) - (pSlash - pRtpMap);
                         *ppEncodingParams = (char *)MP4Calloc(length + 1);
                         strncpy(*ppEncodingParams, pSlash, length);
EOF

test -f taglib_CVE-2018-11439.patch ||
  cat >taglib_CVE-2018-11439.patch <<"EOF"
From 2c4ae870ec086f2ddd21a47861a3709c36faac45 Mon Sep 17 00:00:00 2001
From: Scott Gayou <github.scott@gmail.com>
Date: Tue, 9 Oct 2018 18:46:55 -0500
Subject: Fixed OOB read when loading invalid ogg flac file. (#868) (#869)

CVE-2018-11439 is caused by a failure to check the minimum length
of a ogg flac header. This header is detailed in full at:
https://xiph.org/flac/ogg_mapping.html. Added more strict checking
for entire header.

diff --git a/taglib/ogg/flac/oggflacfile.cpp b/taglib/ogg/flac/oggflacfile.cpp
index 53d04508..07ea9dcc 100644
--- a/taglib/ogg/flac/oggflacfile.cpp
+++ b/taglib/ogg/flac/oggflacfile.cpp
@@ -231,11 +231,21 @@ void Ogg::FLAC::File::scan()
 
   if(!metadataHeader.startsWith("fLaC"))  {
     // FLAC 1.1.2+
+    // See https://xiph.org/flac/ogg_mapping.html for the header specification.
+    if(metadataHeader.size() < 13)
+      return;
+
+    if(metadataHeader[0] != 0x7f)
+      return;
+
     if(metadataHeader.mid(1, 4) != "FLAC")
       return;
 
-    if(metadataHeader[5] != 1)
-      return; // not version 1
+    if(metadataHeader[5] != 1 && metadataHeader[6] != 0)
+      return; // not version 1.0
+
+    if(metadataHeader.mid(9, 4) != "fLaC")
+      return;
 
     metadataHeader = metadataHeader.mid(13);
   }
EOF

test -f taglib_ogg_packet_loss.patch ||
  cat >taglib_ogg_packet_loss.patch <<"EOF"
From 9336c82da3a04552168f208cd7a5fa4646701ea4 Mon Sep 17 00:00:00 2001
From: Tsuda Kageyu <tsuda.kageyu@gmail.com>
Date: Thu, 1 Dec 2016 11:32:01 +0900
Subject: Fix possible Ogg packet losses.


diff --git a/taglib/ogg/oggfile.cpp b/taglib/ogg/oggfile.cpp
index 86b0b076..c36e4d46 100644
--- a/taglib/ogg/oggfile.cpp
+++ b/taglib/ogg/oggfile.cpp
@@ -253,7 +253,7 @@ void Ogg::File::writePacket(unsigned int i, const ByteVector &packet)
   ByteVectorList packets = firstPage->packets();
   packets[i - firstPage->firstPacketIndex()] = packet;
 
-  if(firstPage != lastPage && lastPage->packetCount() > 2) {
+  if(firstPage != lastPage && lastPage->packetCount() > 1) {
     ByteVectorList lastPagePackets = lastPage->packets();
     lastPagePackets.erase(lastPagePackets.begin());
     packets.append(lastPagePackets);
EOF

test -f taglib_aiff_padding.patch ||
  cat >taglib_aiff_padding.patch <<"EOF"
--- a/taglib/riff/rifffile.cpp
+++ b/taglib/riff/rifffile.cpp
@@ -325,9 +325,20 @@ void RIFF::File::read()
     if(offset & 1) {
       seek(offset);
       const ByteVector iByte = readBlock(1);
-      if(iByte.size() == 1 && iByte[0] == '\0') {
-        chunk.padding = 1;
-        offset++;
+      if(iByte.size() == 1) {
+        bool skipPadding = iByte[0] == '\0';
+        if(!skipPadding) {
+          // Padding byte is not zero, check if it is good to ignore it
+          const ByteVector fourCcAfterPadding = readBlock(4);
+          if(isValidChunkName(fourCcAfterPadding)) {
+            // Use the padding, it is followed by a valid chunk name.
+            skipPadding = true;
+          }
+        }
+        if(skipPadding) {
+          chunk.padding = 1;
+          offset++;
+        }
       }
     }
 
EOF

test -f vorbis_alloc_on_heap.patch ||
cat >vorbis_alloc_on_heap.patch <<"EOF"
From: Ralph Giles <giles@thaumas.net>
Date: Tue, 13 Oct 2015 22:32:59 +0000 (-0700)
Subject: Allocate comment temporaries on the heap.
X-Git-Url: https://git.xiph.org/?p=vorbis.git;a=commitdiff_plain;h=c75b3b1282de1010883aa1391bc8ea31dc8ac98e

Allocate comment temporaries on the heap.

Use malloc/free instead of the more convenient alloca for
comment data. Album art can easily be larger than the local
stack limit and crash the process.

Thanks to Robert Kausch for the suggestion.
---

diff --git a/lib/info.c b/lib/info.c
index e447a0c..b8f25ee 100644
--- a/lib/info.c
+++ b/lib/info.c
@@ -65,11 +65,13 @@ void vorbis_comment_add(vorbis_comment *vc,const char *comment){
 }
 
 void vorbis_comment_add_tag(vorbis_comment *vc, const char *tag, const char *contents){
-  char *comment=alloca(strlen(tag)+strlen(contents)+2); /* +2 for = and \0 */
+  /* Length for key and value +2 for = and \0 */
+  char *comment=_ogg_malloc(strlen(tag)+strlen(contents)+2);
   strcpy(comment, tag);
   strcat(comment, "=");
   strcat(comment, contents);
   vorbis_comment_add(vc, comment);
+  _ogg_free(comment);
 }
 
 /* This is more or less the same as strncasecmp - but that doesn't exist
@@ -88,27 +90,30 @@ char *vorbis_comment_query(vorbis_comment *vc, const char *tag, int count){
   long i;
   int found = 0;
   int taglen = strlen(tag)+1; /* +1 for the = we append */
-  char *fulltag = alloca(taglen+ 1);
+  char *fulltag = _ogg_malloc(taglen+1);
 
   strcpy(fulltag, tag);
   strcat(fulltag, "=");
 
   for(i=0;i<vc->comments;i++){
     if(!tagcompare(vc->user_comments[i], fulltag, taglen)){
-      if(count == found)
+      if(count == found) {
         /* We return a pointer to the data, not a copy */
-              return vc->user_comments[i] + taglen;
-      else
+        _ogg_free(fulltag);
+        return vc->user_comments[i] + taglen;
+      } else {
         found++;
+      }
     }
   }
+  _ogg_free(fulltag);
   return NULL; /* didn't find anything */
 }
 
 int vorbis_comment_query_count(vorbis_comment *vc, const char *tag){
   int i,count=0;
   int taglen = strlen(tag)+1; /* +1 for the = we append */
-  char *fulltag = alloca(taglen+1);
+  char *fulltag = _ogg_malloc(taglen+1);
   strcpy(fulltag,tag);
   strcat(fulltag, "=");
 
@@ -117,6 +122,7 @@ int vorbis_comment_query_count(vorbis_comment *vc, const char *tag){
       count++;
   }
 
+  _ogg_free(fulltag);
   return count;
 }
 
EOF

test -f ffmpeg_mingw.patch ||
  cat >ffmpeg_mingw.patch <<"EOF"
From a64839189622f2a4cc3c62168ae5037b6aab6992 Mon Sep 17 00:00:00 2001
From: Tobias Rapp <t.rapp@noa-archive.com>
Date: Mon, 29 Aug 2016 15:25:58 +0200
Subject: cmdutils: fix implicit declaration of SetDllDirectory function

Signed-off-by: Tobias Rapp <t.rapp@noa-archive.com>
Signed-off-by: James Almer <jamrial@gmail.com>

diff --git a/cmdutils.c b/cmdutils.c
index 6960f8c..44f44cd 100644
--- a/cmdutils.c
+++ b/cmdutils.c
@@ -61,6 +61,9 @@
 #include <sys/time.h>
 #include <sys/resource.h>
 #endif
+#ifdef _WIN32
+#include <windows.h>
+#endif
 
 static int init_report(const char *env);
EOF

if test "$compiler" = "cross-android"; then
  test -f openssl_android.patch ||
    cat >openssl_android.patch <<"EOF"
--- Setenv-android.sh	2018-02-10 13:37:18.181017337 +0100
+++ Setenv-android.sh.new	2018-02-10 13:33:11.661974672 +0100
@@ -102,7 +102,7 @@
 # https://android.googlesource.com/platform/ndk/+/ics-mr0/docs/STANDALONE-TOOLCHAIN.html
 
 ANDROID_TOOLCHAIN=""
-for host in "linux-x86_64" "linux-x86" "darwin-x86_64" "darwin-x86"
+for host in "linux-x86_64" "linux-x86" "darwin-x86_64" "darwin-x86" "windows-x86" "windows-x86_64"
 do
   if [ -d "$ANDROID_NDK_ROOT/toolchains/$_ANDROID_EABI/prebuilt/$host/bin" ]; then
     ANDROID_TOOLCHAIN="$ANDROID_NDK_ROOT/toolchains/$_ANDROID_EABI/prebuilt/$host/bin"
@@ -124,6 +124,9 @@
 	arch-x86)	  
       ANDROID_TOOLS="i686-linux-android-gcc i686-linux-android-ranlib i686-linux-android-ld"
 	  ;;	  
+	arch-arm64)
+      ANDROID_TOOLS="aarch64-linux-android-gcc aarch64-linux-android-ranlib aarch64-linux-android-ld"
+	  ;;
 	*)
 	  echo "ERROR ERROR ERROR"
 	  ;;
@@ -206,6 +209,14 @@
 	export CROSS_COMPILE="i686-linux-android-"
 fi
 
+if [ "$_ANDROID_ARCH" == "arch-arm64" ]; then
+	export MACHINE=armv8
+	export RELEASE=2.6.37
+	export SYSTEM=android64
+	export ARCH=arm
+	export CROSS_COMPILE="aarch64-linux-android-"
+fi
+
 # For the Android toolchain
 # https://android.googlesource.com/platform/ndk/+/ics-mr0/docs/STANDALONE-TOOLCHAIN.html
 export ANDROID_SYSROOT="$ANDROID_NDK_ROOT/platforms/$_ANDROID_API/$_ANDROID_ARCH"
EOF
fi # cross-android

cd ..


# Extract and patch sources

if ! test -d taglib-${taglib_version}; then
  echo "### Extracting taglib"

  tar xzf source/taglib-${taglib_version}.tar.gz
  cd taglib-${taglib_version}/
  taglib_nr=${taglib_version:0:3}
  if test $taglib_nr = "1.1"; then
    taglib_nr=${taglib_version:0:4}
    taglib_nr=${taglib_nr/./}
  else
    taglib_nr=${taglib_nr/./0}
  fi
  if test $taglib_nr -ge 108; then
    patch -p1 <../source/taglib-msvc.patch
  else
    sed -i 's/^ADD_SUBDIRECTORY(bindings)/#ADD_SUBDIRECTORY(bindings)/' ./CMakeLists.txt
    sed -i 's/^ADD_LIBRARY(tag SHARED/ADD_LIBRARY(tag STATIC/' ./taglib/CMakeLists.txt
  fi
  if test $taglib_nr -ge 111; then
    taglib_static_option=-DBUILD_SHARED_LIBS=OFF
  else
    taglib_static_option=-DENABLE_STATIC=ON
  fi
  if test "${taglib_version}" = "1.9.1"; then
    patch -p1 <../source/taglib_bvreplace.patch
  fi
  if test "${taglib_version}" = "1.11.1"; then
    patch -p1 <../source/taglib_mp4shwm.patch
    patch -p1 <../source/taglib_CVE-2017-12678.patch
    patch -p1 <../source/taglib_cmID_purl_egid.patch.patch
    patch -p1 <../source/taglib_CVE-2018-11439.patch
    patch -p1 <../source/taglib_ogg_packet_loss.patch
    patch -p1 <../source/taglib_aiff_padding.patch
    patch -p1 <../source/taglib_grp1.patch
    patch -p1 <../source/taglib_oggbitrate.patch
  fi
  if test "$cross_host" = "x86_64-w64-mingw32"; then
    patch -p1 <../source/taglib_large_file.patch
  fi
  cd ..
fi

if test "$compiler" != "cross-android"; then

  if test -n "$ZLIB_ROOT_PATH"; then
    if ! test -d zlib-${zlib_version}; then
      echo "### Extracting zlib"

      tar xzf source/zlib_${zlib_version}.dfsg.orig.tar.gz
      cd zlib-${zlib_version}/

      tar xJf ../source/zlib_${zlib_version}.dfsg-${zlib_patchlevel}.debian.tar.xz || true
      echo Can be ignored: Cannot create symlink to debian.series
      for f in $(cat debian/patches/debian.series); do patch -p1 <debian/patches/$f; done
      cd ..
    fi
  fi

  if ! test -d libogg-${libogg_version}; then
    echo "### Extracting libogg"

    tar xzf source/libogg_${libogg_version}.orig.tar.gz
    cd libogg-${libogg_version}/
    gunzip -c ../source/libogg_${libogg_version}-${libogg_patchlevel}.diff.gz | patch -p1
    cd ..
  fi

  if ! test -d libvorbis-${libvorbis_version}; then
    echo "### Extracting libvorbis"

    tar xzf source/libvorbis_${libvorbis_version}.orig.tar.gz
    cd libvorbis-${libvorbis_version}/
    tar xJf ../source/libvorbis_${libvorbis_version}-${libvorbis_patchlevel}.debian.tar.xz
    for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    if test $libvorbis_version = "1.3.5"; then
      patch -p1 <../source/vorbis_alloc_on_heap.patch
    fi
    test -f win32/VS2010/libogg.props.orig || mv win32/VS2010/libogg.props win32/VS2010/libogg.props.orig
    sed "s/<LIBOGG_VERSION>1.2.0</<LIBOGG_VERSION>$libogg_version</" win32/VS2010/libogg.props.orig >win32/VS2010/libogg.props
    cd ..
  fi

  if ! test -d flac-${libflac_version}; then
    echo "### Extracting libflac"

    tar xJf source/flac_${libflac_version}.orig.tar.xz
    cd flac-${libflac_version}/
    tar xJf ../source/flac_${libflac_version}-${libflac_patchlevel}.debian.tar.xz
    for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    patch -p1 <../source/flac_1.2.1_size_t_max_patch.diff
    if test $kernel = "Darwin"; then
      patch -p1 <../source/fink_flac.patch
      patch -p0 <patches/nasm.h.patch
    fi
    cd ..
  fi

  if ! test -d id3lib-${id3lib_version}; then
    echo "### Extracting id3lib"

    tar xzf source/id3lib3.8.3_${id3lib_version}.orig.tar.gz
    cd id3lib-${id3lib_version}/
    tar xJf ../source/id3lib3.8.3_${id3lib_version}-${id3lib_patchlevel}.debian.tar.xz
    for f in $(cat debian/patches/series); do patch --binary -p1 <debian/patches/$f; done
    patch -p1 <../source/id3lib-3.8.3_mingw.patch
    patch -p1 <../source/id3lib-3.8.3_wintempfile.patch
    test -f makefile.win32.orig || mv makefile.win32 makefile.win32.orig
    sed 's/-W3 -WX -GX/-W3 -EHsc/; s/-MD -D "WIN32" -D "_DEBUG"/-MDd -D "WIN32" -D "_DEBUG"/' makefile.win32.orig >makefile.win32
    cd ..
  fi

  if test -n "${ffmpeg_version}"; then
    if ! test -d ffmpeg-${ffmpeg_version}; then
      echo "### Extracting ffmpeg"

      tar xJf source/ffmpeg_${ffmpeg_version}.orig.tar.xz || true
      cd ffmpeg-${ffmpeg_version}/
      tar xJf ../source/ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz
      for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
      if test $ffmpeg_version = "3.1.3"; then
        patch -p1 <../source/ffmpeg_mingw.patch
      fi
      cd ..
    fi
  else
    if test "${libav_version%.*}" = "0.8"; then
      if ! test -d libav-${libav_version}; then
        echo "### Extracting libav"

        tar xzf source/libav_${libav_version}.orig.tar.gz
        cd libav-${libav_version}/
        tar xzf ../source/libav_${libav_version}-${libav_patchlevel}.debian.tar.gz
        oldifs=$IFS
        IFS='
'
        for f in $(cat debian/patches/series); do
          if test "${f:0:1}" != "#"; then
            patch -p1 <debian/patches/$f
          fi
        done
        IFS=$oldifs
        cd ..
      fi
    else
      if ! test -d libav-${libav_version}; then
        echo "### Extracting libav"

        tar xJf source/libav_${libav_version}.orig.tar.xz || true
        echo Can be ignored: Cannot create symlink to README.md
        cd libav-${libav_version}/
        tar xJf ../source/libav_${libav_version}-${libav_patchlevel}.debian.tar.xz
        oldifs=$IFS
        IFS='
'
        for f in $(cat debian/patches/series); do
          if test "${f:0:1}" != "#"; then
            patch -p1 <debian/patches/$f
          fi
        done
        IFS=$oldifs
        cd ..
      fi
    fi
  fi

  if ! test -d chromaprint-${chromaprint_version}; then
    echo "### Extracting chromaprint"

    tar xzf source/chromaprint_${chromaprint_version}.orig.tar.gz
    cd chromaprint-${chromaprint_version}/
    tar xJf ../source/chromaprint_${chromaprint_version}-${chromaprint_patchlevel}.debian.tar.xz
    for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    cd ..
  fi

  if ! test -d mp4v2-${mp4v2_version}; then
    echo "### Extracting mp4v2"

    tar xjf source/mp4v2_${mp4v2_version}~dfsg0.orig.tar.bz2
    cd mp4v2-${mp4v2_version}/
    tar xJf ../source/mp4v2_${mp4v2_version}~dfsg0-${mp4v2_patchlevel}.debian.tar.xz
    for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    if test $kernel = "MINGW" || test "$compiler" = "cross-mingw"; then
      patch -p1 <../source/mp4v2_win32.patch
      if test -z "${cross_host##x86_64*}"; then
        sed -i '/^#   define _USE_32BIT_TIME_T/ s#^#//#' libplatform/platform_win32.h
      fi
    fi
    patch -p1 <../source/mp4v2_clang6.patch
    cd ..
  fi

fi # !cross-android

if test "$compiler" = "cross-android"; then

  if ! test -d openssl-${openssl_version}; then
    echo "### Extracting openssl"

    tar xzf source/openssl-${openssl_version}.tar.gz
    cp source/Setenv-android.sh openssl-${openssl_version}/
    cd openssl-${openssl_version}/
    sed -i 's/\r$//' Setenv-android.sh
    test -n "$ANDROID_NDK_ROOT" && \
      sed -i "s#^ANDROID_NDK_ROOT=.*#ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT#" \
        Setenv-android.sh
    chmod +x Setenv-android.sh
    patch -p0 <../source/openssl_android.patch
    cd ..
  fi

elif test "$compiler" = "gcc-self-contained" || test "$compiler" = "gcc-debug" \
     || ( ( test "$compiler" = "cross-mingw" || test "$kernel" = "MINGW" ) && test "${openssl_version:0:3}" != "1.0" ); then

  if ! test -d openssl-${openssl_version}; then
    echo "### Extracting openssl"
    tar xzf source/openssl-${openssl_version}.tar.gz
  fi

fi # cross-android

# Build from sources

test -d bin || mkdir bin

_chocoInstall=${ChocolateyInstall//\\/\/}
_chocoInstall=${_chocoInstall/C:/\/c}
for d in "$DOCBOOK_XSL_DIR" /usr/share/xml/docbook/stylesheet/nwalsh /usr/share/xml/docbook/xsl-stylesheets-* /usr/local/Cellar/docbook-xsl/*/docbook-xsl /opt/local/share/xsl/docbook-xsl $_chocoInstall/lib/docbook-bundle/docbook-xsl-*; do
  if test -e $d/xhtml/docbook.xsl; then
    _docbook_xsl_dir=$d
    break
  fi
done

if test "$compiler" = "cross-android"; then

  _java_root=${JAVA_HOME:-/usr/lib/jvm/java-8-openjdk-amd64}
  _android_sdk_root=${ANDROID_SDK_ROOT:-/opt/android/sdk}
  _android_ndk_root=${ANDROID_NDK_ROOT:-$_android_sdk_root/ndk-bundle}
  _android_platform=${ANDROID_PLATFORM:-23}
  _android_ccache=$(which ccache || true)
  _android_qt_root=${QTPREFIX:-/opt/qt5/5.9.7/android_armv7}
  _android_toolchain_cmake=$_android_ndk_root/build/cmake/android.toolchain.cmake
  test -f $_android_toolchain_cmake ||
    _android_toolchain_cmake=$srcdir/android/qt-android-cmake/toolchain/android.toolchain.cmake
  test -f $_android_qt_root/bin/qmake ||
    _android_qt_root=/opt/qt5/5.9.7/android_armv7
  if test -z "${_android_qt_root%%*x86}"; then
    _android_abi=x86
    _android_prefix=i686-linux-android
  else
    _android_abi=armeabi-v7a
    _android_prefix=arm-linux-androideabi
  fi
  if test ! -d openssl-${openssl_version}/inst; then
    echo "### Building OpenSSL"

    cd openssl-${openssl_version}/
    if test "$_android_abi" = "x86"; then
      sed -i 's/^_ANDROID_EABI=.*$/_ANDROID_EABI=x86-4.9/; s/^_ANDROID_ARCH=.*$/_ANDROID_ARCH=arch-x86/' Setenv-android.sh
    else
      sed -i 's/^_ANDROID_EABI=.*$/_ANDROID_EABI=arm-linux-androideabi-4.9/; s/^_ANDROID_ARCH=.*$/_ANDROID_ARCH=arch-arm/' Setenv-android.sh
    fi
    sed -i "s#^_ANDROID_NDK=.*#ANDROID_NDK_ROOT=$_android_ndk_root#" Setenv-android.sh
    sed -i '/FIPS_SIG location/,/^fi$/ d' Setenv-android.sh
    if test -d $_android_ndk_root/toolchains/llvm; then
      sed -i 's/^_ANDROID_EABI=.*$/_ANDROID_EABI=llvm/' Setenv-android.sh
    fi
    . ./Setenv-android.sh
    if test "${openssl_version:0:3}" = "1.0"; then
      ./Configure shared android
    else
      ANDROID_NDK_HOME=$ANDROID_NDK_ROOT ./Configure shared android-armeabi
    fi
    if test -d $_android_ndk_root/toolchains/llvm; then
      if test "$_android_abi" = "x86"; then
        sed -i 's/^CC=.*$/CC= i686-linux-android16-clang/; s/ -mandroid//' Makefile
      else
        sed -i 's/^CC=.*$/CC= armv7a-linux-androideabi16-clang/; s/ -mandroid//' Makefile
      fi
    fi

    if test "${openssl_version:0:3}" = "1.0"; then
      make CALC_VERSIONS="SHLIB_COMPAT=; SHLIB_SOVER=" build_libs
    else
      make ANDROID_NDK_HOME=$ANDROID_NDK_ROOT SHLIB_VERSION_NUMBER= SHLIB_EXT=.so build_libs
    fi
    mkdir -p inst/usr/local/lib
    cp --dereference libssl.so libcrypto.so inst/usr/local/lib/
    $_android_prefix-strip -s inst/usr/local/lib/*.so
    cd inst
    tar czf ../../bin/openssl-${openssl_version}.tgz usr
    cd ../..
    tar xmzf bin/openssl-${openssl_version}.tgz -C $BUILDROOT
  fi

  if ! test -f $BUILDROOT/usr/local/lib/libtag.a; then
    echo "### Building taglib"

    cd taglib-${taglib_version}/
    cmake -DWITH_ASF=ON -DWITH_MP4=ON $taglib_static_option -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$BUILDROOT/usr/local -DANDROID_NDK=$_android_ndk_root -DANDROID_ABI=$_android_abi -DCMAKE_TOOLCHAIN_FILE=$_android_toolchain_cmake -DANDROID_PLATFORM=$_android_platform -DANDROID_CCACHE=$_android_ccache -DCMAKE_MAKE_PROGRAM=make
    make install
    cd ..
  fi

  if ! test -d kid3; then
    echo "### Creating kid3 build directory"
    mkdir kid3
    test -e $HOME/Development/ufleisch-release-key.keystore && cp -s $HOME/Development/ufleisch-release-key.keystore kid3/
    cat >kid3/build.sh <<EOF
#!/bin/bash
_java_root=$_java_root
_android_sdk_root=$_android_sdk_root
_android_ndk_root=$_android_ndk_root
_android_platform=$_android_platform
_android_ccache=$_android_ccache
_android_abi=$_android_abi
_android_qt_root=$_android_qt_root
_android_keystore_path=\$(pwd)/ufleisch-release-key.keystore
_android_keystore_alias=ufleisch_android
if ! test -f "\$_android_keystore_path"; then
  _android_keystore_path=
  _android_keystore_alias=
fi
_buildprefix=\$(cd ..; pwd)/buildroot/usr/local
# Pass -DQT_ANDROID_USE_GRADLE=ON to use Gradle instead of ANT.
cmake -DJAVA_HOME=\$_java_root -DQT_ANDROID_SDK_ROOT=\$_android_sdk_root -DANDROID_NDK=\$_android_ndk_root -DAPK_ALL_TARGET=OFF -DANDROID_ABI=\$_android_abi -DANDROID_EXTRA_LIBS_DIR=\$_buildprefix/lib -DANDROID_KEYSTORE_PATH=\$_android_keystore_path -DANDROID_KEYSTORE_ALIAS=\$_android_keystore_alias -DCMAKE_TOOLCHAIN_FILE=$_android_toolchain_cmake -DANDROID_PLATFORM=$_android_platform -DANDROID_CCACHE=$_android_ccache -DQT_QMAKE_EXECUTABLE=\$_android_qt_root/bin/qmake -DCMAKE_BUILD_TYPE=Release -DDOCBOOK_XSL_DIR=${_docbook_xsl_dir} -DPYTHON_EXECUTABLE=/usr/bin/python -DXSLTPROC=/usr/bin/xsltproc -DGZIP_EXECUTABLE=/bin/gzip -DTAGLIBCONFIG_EXECUTABLE=\$_buildprefix/bin/taglib-config -DCMAKE_MAKE_PROGRAM=make $srcdir
EOF
    chmod +x kid3/build.sh
  fi

elif test "$compiler" = "msvc"; then

  if test ! -d libogg-${libogg_version}/inst; then
    echo "### Building libogg"

    cd libogg-${libogg_version}/
    $COMSPEC /c "\"\"%VS110COMNTOOLS%vsvars32.bat\"\" && msbuild win32\VS2010\libogg_static.sln /p:Configuration=Debug;Platform=Win32"
    $COMSPEC /c "\"\"%VS110COMNTOOLS%vsvars32.bat\"\" && msbuild win32\VS2010\libogg_static.sln /p:Configuration=Release;Platform=Win32"
    mkdir -p inst/include/ogg inst/lib/Debug inst/lib/Release
    cp win32/VS2010/Win32/Debug/libogg_static.lib inst/lib/Debug/
    cp win32/VS2010/Win32/Release/libogg_static.lib inst/lib/Release/
    cp include/ogg/*.h inst/include/ogg/
    cd inst
    tar czf ../../bin/libogg-${libogg_version}.tgz include lib
    cd ../..
    tar xmzf bin/libogg-${libogg_version}.tgz $BUILDROOT
  fi

  if test ! -d libvorbis-${libvorbis_version}/inst; then
    echo "### Building libvorbis"

    cd libvorbis-${libvorbis_version}/
    $COMSPEC /c "\"\"%VS110COMNTOOLS%vsvars32.bat\"\" && msbuild win32\VS2010\vorbis_static.sln /p:Configuration=Debug;Platform=Win32"
    $COMSPEC /c "\"\"%VS110COMNTOOLS%vsvars32.bat\"\" && msbuild win32\VS2010\vorbis_static.sln /p:Configuration=Release;Platform=Win32"
    mkdir -p inst/include/vorbis inst/lib/Debug inst/lib/Release
    cp win32/VS2010/Win32/Debug/*.lib inst/lib/Debug/
    cp win32/VS2010/Win32/Release/*.lib inst/lib/Release/
    cp include/vorbis/*.h inst/include/vorbis/
    cd inst
    tar czf ../../bin/libvorbis-${libvorbis_version}.tgz include lib
    cd ../..
    tar xmzf bin/libvorbis-${libvorbis_version}.tgz -C $BUILDROOT
  fi

  if test ! -d id3lib-${id3lib_version}/inst; then
    echo "### Building id3lib"

    cd id3lib-${id3lib_version}/
    test -f config.h || sed 's/^#define CXX_HAS_BUGGY_FOR_LOOPS 1/\/\/#define CXX_HAS_BUGGY_FOR_LOOPS 1/' config.h.win32 >config.h
    $COMSPEC /c "\"\"%VS110COMNTOOLS%vsvars32.bat\"\" && nmake -f makefile.win32 DEBUG=1"
    $COMSPEC /c "\"\"%VS110COMNTOOLS%vsvars32.bat\"\" && nmake -f makefile.win32"
    mkdir -p inst/include inst/lib/Debug inst/lib/Release
    cp -a include/id3* inst/include
    cp id3libd.lib inst/lib/Debug/id3lib.lib
    cp id3lib.lib inst/lib/Release/
    cd inst
    tar czf ../../bin/id3lib-${id3lib_version}.tgz include lib
    cd ../..
    tar xmzf bin/id3lib-${id3lib_version}.tgz -C $BUILDROOT
  fi

  if test ! -d taglib-${taglib_version}/inst; then
    echo "### Building taglib"

    cd taglib-${taglib_version}/
    test -f taglib.sln || cmake -G "Visual Studio 11" -DWITH_ASF=ON -DWITH_MP4=ON $taglib_static_option -DCMAKE_INSTALL_PREFIX=
    mkdir -p instd
    DESTDIR=instd cmake --build . --config Debug --target install
    mkdir -p inst
    DESTDIR=inst cmake --build . --config Release --target install
    mv inst/lib inst/Release
    mv instd/lib inst/Debug
    mkdir -p inst/lib
    mv inst/Debug inst/Release inst/lib/
    rm -rf instd
    cd inst
    tar czf ../../bin/taglib-${taglib_version}.tgz include lib
    cd ../..
    tar xmzf bin/taglib-${taglib_version}.tgz -C $BUILDROOT
  fi

else #  cross-android, msvc

  if test "$1" = "clean"; then
    for d in zlib-${zlib_version} libogg-${libogg_version} \
             libvorbis-${libvorbis_version} flac-${libflac_version} \
             id3lib-${id3lib_version} taglib-${taglib_version} \
             ${ffmpeg_dir} chromaprint-${chromaprint_version} \
             mp4v2-${mp4v2_version}; do
      test -d $d/inst && rm -rf $d/inst
    done
  fi

  if ( test "$compiler" = "gcc-self-contained" || test "$compiler" = "gcc-debug" ) && test ! -d openssl-${openssl_version}/inst; then
    echo "### Building OpenSSL"

    cd openssl-${openssl_version}
    ./Configure shared enable-ec_nistp_64_gcc_128 linux-x86_64 -Wa,--noexecstack
    make depend || true
    make build_libs
    mkdir -p inst/usr/local/ssl
    cp --dereference libssl.so libcrypto.so inst/usr/local/ssl/
    strip -s inst/usr/local/ssl/*.so
    cd inst
    tar czf ../../bin/openssl-${openssl_version}.tgz usr
    cd ../..
    tar xmzf bin/openssl-${openssl_version}.tgz -C $BUILDROOT

  elif ( ( test "$compiler" = "cross-mingw" || test "$kernel" = "MINGW" ) && test "${openssl_version:0:3}" != "1.0" ) \
       && test ! -d openssl-${openssl_version}/inst; then
    echo "### Building OpenSSL"

    cd openssl-${openssl_version}
    if test "$cross_host" = "x86_64-w64-mingw32" || [[ $(uname) =~ ^MINGW64 ]]; then
      _target=mingw64
    else
      _target=mingw
    fi
    if test -n "${cross_host}"; then
      _crossprefix=${cross_host}-
    else
      _crossprefix=
    fi
    ./Configure shared enable-ec_nistp_64_gcc_128 $_target --cross-compile-prefix=$_crossprefix
    make depend || true
    make build_libs
    mkdir -p inst/usr/local/ssl
    cp lib{ssl,crypto}*.dll inst/usr/local/ssl/
    ${_crossprefix}strip -s inst/usr/local/ssl/*.dll
    cd inst
    tar czf ../../bin/openssl-${openssl_version}.tgz usr
    cd ../..
    tar xmzf bin/openssl-${openssl_version}.tgz -C $BUILDROOT
  fi

  if test -n "$ZLIB_ROOT_PATH" && test ! -d zlib-${zlib_version}/inst; then
    echo "### Building zlib"

    cd zlib-${zlib_version}/
    if test $kernel = "MINGW"; then
      make -f win32/Makefile.gcc
      make install -f win32/Makefile.gcc INCLUDE_PATH=`pwd`/inst/usr/local/include LIBRARY_PATH=`pwd`/inst/usr/local/lib BINARY_PATH=`pwd`/inst/usr/local/bin
    elif test "$compiler" = "cross-mingw"; then
      make -f win32/Makefile.gcc LOC=-g PREFIX=${cross_host}-
      make install -f win32/Makefile.gcc INCLUDE_PATH=`pwd`/inst/usr/local/include LIBRARY_PATH=`pwd`/inst/usr/local/lib BINARY_PATH=`pwd`/inst/usr/local/bin
    else
      CFLAGS="$CFLAGS -g -O3 -Wall -DNO_FSEEKO" ./configure --static
      sed 's/LIBS=$(STATICLIB) $(SHAREDLIB) $(SHAREDLIBV)/LIBS=$(STATICLIB)/' Makefile >Makefile.inst
      mkdir -p inst/usr/local
      make install -f Makefile.inst prefix=`pwd`/inst/usr/local
    fi
    cd inst
    tar czf ../../bin/zlib-${zlib_version}.tgz usr
    cd ../..
    tar xmzf bin/zlib-${zlib_version}.tgz -C $BUILDROOT
  fi

  if test ! -d libogg-${libogg_version}/inst; then
    echo "### Building libogg"

    cd libogg-${libogg_version}/
    test -f Makefile || ./configure --enable-shared=no --enable-static=yes $CONFIGURE_OPTIONS
    make
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    cd inst
    tar czf ../../bin/libogg-${libogg_version}.tgz usr
    cd ../..
    tar xmzf bin/libogg-${libogg_version}.tgz -C $BUILDROOT
  fi

  if test ! -d libvorbis-${libvorbis_version}/inst; then
    echo "### Building libvorbis"

    cd libvorbis-${libvorbis_version}/
    if test "$compiler" = "cross-mingw"; then
      test -f Makefile || CFLAGS="$CFLAGS -g" PKG_CONFIG= ./configure --enable-shared=no --enable-static=yes --with-ogg=$thisdir/libogg-$libogg_version/inst/usr/local $CONFIGURE_OPTIONS
    else
      test -f Makefile || CFLAGS="$CFLAGS -g" ./configure --enable-shared=no --enable-static=yes --with-ogg=$thisdir/libogg-$libogg_version/inst/usr/local $CONFIGURE_OPTIONS
    fi
    make
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    cd inst
    tar czf ../../bin/libvorbis-${libvorbis_version}.tgz usr
    cd ../..
    tar xmzf bin/libvorbis-${libvorbis_version}.tgz -C $BUILDROOT
  fi

  if test ! -d flac-${libflac_version}/inst; then
    echo "### Building libflac"

    cd flac-${libflac_version}/
    autoreconf -i
    configure_args="--enable-shared=no --enable-static=yes --with-ogg=$thisdir/libogg-$libogg_version/inst/usr/local --disable-thorough-tests --disable-doxygen-docs --disable-xmms-plugin $FLAC_BUILD_OPTION $CONFIGURE_OPTIONS"
    if test $kernel = "Darwin"; then
      configure_args="$configure_args --disable-asm-optimizations"
    fi
    test -f Makefile || ./configure $configure_args
    # On msys32, an error "changed before entering" occurred, can be fixed by
    # modifying /usr/share/perl5/core_perl/File/Path.pm
    # my $Need_Stat_Check = !($^O eq 'MSWin32' || $^O eq 'msys');
    make V=1
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    cd inst
    tar czf ../../bin/flac-${libflac_version}.tgz usr
    cd ../..
    tar xmzf bin/flac-${libflac_version}.tgz -C $BUILDROOT
  fi

  if test ! -d id3lib-${id3lib_version}/inst; then
    echo "### Building id3lib"

    cd id3lib-${id3lib_version}/
    if test $kernel = "MINGW" || test "$compiler" = "cross-mingw"; then
      sed -i 's/^@ID3_NEEDDEBUG_TRUE@ID3_DEBUG_LIBS = -lcwd -lbfd -liberty$/@ID3_NEEDDEBUG_TRUE@ID3_DEBUG_LIBS =/' examples/Makefile.in
    fi
    autoconf
    configure_args="--enable-shared=no --enable-static=yes $ID3LIB_BUILD_OPTION $CONFIGURE_OPTIONS"
    if test $kernel = "MINGW"; then
      configure_args="$configure_args --build=mingw32"
    fi
    test -f Makefile || CPPFLAGS=-I/usr/local/include LDFLAGS="$LDFLAGS -L/usr/local/lib" ./configure $configure_args
    SED=sed make
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    cd inst
    tar czf ../../bin/id3lib-${id3lib_version}.tgz usr
    cd ../..
    tar xmzf bin/id3lib-${id3lib_version}.tgz -C $BUILDROOT
  fi

  if test ! -d taglib-${taglib_version}/inst; then
    echo "### Building taglib"

    cd taglib-${taglib_version}/
    test -f Makefile || eval cmake -DWITH_ASF=ON -DWITH_MP4=ON -DINCLUDE_DIRECTORIES=/usr/local/include -DLINK_DIRECTORIES=/usr/local/lib $taglib_static_option $TAGLIB_ZLIB_ROOT_OPTION $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
    make VERBOSE=1
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    fixcmakeinst
    cd inst
    tar czf ../../bin/taglib-${taglib_version}.tgz usr
    cd ../..
    tar xmzf bin/taglib-${taglib_version}.tgz -C $BUILDROOT
  fi

  if test ! -d ${ffmpeg_dir}/inst; then
    echo "### Building ffmpeg"

    if test "${libav_version%.*}" = "0.8"; then
      cd ${ffmpeg_dir}
      # configure needs yasm and pr
      # On msys, make >= 3.81 is needed.
      # Most options taken from
      # http://oxygene.sk/lukas/2011/04/minimal-audio-only-ffmpeg-build-with-mingw32/
      # Disable-sse avoids a SEGFAULT under MinGW.
      # Later versions (tested with libav-HEAD-5d2be71) do not have
      # --enable-ffmpeg and additionally need --disable-mmx --disable-mmxext.
      # The two --disable-hwaccel were added for MinGW-builds GCC 4.7.2.
      if test "$compiler" = "cross-mingw"; then
        sed -i 's/^\(.*-Werror=missing-prototypes\)/#\1/' ./configure
        AV_CONFIGURE_OPTIONS="--cross-prefix=${cross_host}- --arch=x86 --target-os=mingw32 --sysinclude=/usr/${cross_host}/include"
      fi
      AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS $AV_BUILD_OPTION"
      ./configure \
	      --enable-memalign-hack \
	      --disable-shared \
	      --enable-static \
	      --disable-avdevice \
	      --disable-avfilter \
	      --disable-pthreads \
	      --disable-swscale \
	      --enable-ffmpeg \
	      --disable-network \
	      --disable-muxers \
	      --disable-demuxers \
	      --disable-sse \
	      --disable-doc \
	      --enable-rdft \
	      --enable-demuxer=aac \
	      --enable-demuxer=ac3 \
	      --enable-demuxer=ape \
	      --enable-demuxer=asf \
	      --enable-demuxer=flac \
	      --enable-demuxer=matroska_audio \
	      --enable-demuxer=mp3 \
	      --enable-demuxer=mpc \
	      --enable-demuxer=mov \
	      --enable-demuxer=mpc8 \
	      --enable-demuxer=ogg \
	      --enable-demuxer=tta \
	      --enable-demuxer=wav \
	      --enable-demuxer=wv \
	      --disable-bsfs \
	      --disable-filters \
	      --disable-parsers \
	      --enable-parser=aac \
	      --enable-parser=ac3 \
	      --enable-parser=mpegaudio \
	      --disable-protocols \
	      --enable-protocol=file \
	      --disable-indevs \
	      --disable-outdevs \
	      --disable-encoders \
	      --disable-decoders \
	      --enable-decoder=aac \
	      --enable-decoder=ac3 \
	      --enable-decoder=alac \
	      --enable-decoder=ape \
	      --enable-decoder=flac \
	      --enable-decoder=mp1 \
	      --enable-decoder=mp2 \
	      --enable-decoder=mp3 \
	      --enable-decoder=mpc7 \
	      --enable-decoder=mpc8 \
	      --enable-decoder=tta \
	      --enable-decoder=vorbis \
	      --enable-decoder=wavpack \
	      --enable-decoder=wmav1 \
	      --enable-decoder=wmav2 \
	      --enable-decoder=pcm_alaw \
	      --enable-decoder=pcm_dvd \
	      --enable-decoder=pcm_f32be \
	      --enable-decoder=pcm_f32le \
	      --enable-decoder=pcm_f64be \
	      --enable-decoder=pcm_f64le \
	      --enable-decoder=pcm_s16be \
	      --enable-decoder=pcm_s16le \
	      --enable-decoder=pcm_s16le_planar \
	      --enable-decoder=pcm_s24be \
	      --enable-decoder=pcm_daud \
	      --enable-decoder=pcm_s24le \
	      --enable-decoder=pcm_s32be \
	      --enable-decoder=pcm_s32le \
	      --enable-decoder=pcm_s8 \
	      --enable-decoder=pcm_u16be \
	      --enable-decoder=pcm_u16le \
	      --enable-decoder=pcm_u24be \
	      --enable-decoder=pcm_u24le \
	      --enable-decoder=rawvideo \
	      --disable-hwaccel=h264_dxva2 \
	      --disable-hwaccel=mpeg2_dxva2 $AV_CONFIGURE_OPTIONS
      make
      mkdir -p inst
      make install DESTDIR=`pwd`/inst
      cd inst
      tar czf ../../bin/${ffmpeg_dir}.tgz usr
      cd ../..
      tar xmzf bin/${ffmpeg_dir}.tgz -C $BUILDROOT
    else
      cd ${ffmpeg_dir}
      # configure needs yasm and pr
      # On msys, make >= 3.81 is needed.
      # Most options taken from
      # http://oxygene.sk/lukas/2011/04/minimal-audio-only-ffmpeg-build-with-mingw32/
      # Disable-sse avoids a SEGFAULT under MinGW.
      # Later versions (tested with libav-HEAD-5d2be71) do not have
      # --enable-ffmpeg and additionally need --disable-mmx --disable-mmxext.
      # The two --disable-hwaccel were added for MinGW-builds GCC 4.7.2.
      # The --extra-cflags=-march=i486 is to avoid error "Threading is enabled, but
      # there is no implementation of atomic operations available", libav bug 471.
      if test "$compiler" = "cross-mingw"; then
        # mkstemp is not available when building on Windows
        sed -i 's/check_func  mkstemp/disable  mkstemp/' ./configure
        sed -i 's/^\(.*-Werror=missing-prototypes\)/#\1/' ./configure
        AV_CONFIGURE_OPTIONS="--cross-prefix=${cross_host}- --arch=x86 --target-os=mingw32 --sysinclude=/usr/${cross_host}/include"
        if test -n "${cross_host##x86_64*}"; then
          AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --extra-cflags=-march=i486"
        fi
      elif test $kernel = "MINGW"; then
        # mkstemp is not available when building with mingw from Qt
        sed -i 's/check_func  mkstemp/disable  mkstemp/' ./configure
        if ! [[ $(uname) =~ ^MINGW64 ]]; then
          AV_CONFIGURE_OPTIONS="--extra-cflags=-march=i486"
        fi
        if test $(uname) = "MSYS_NT-6.1"; then
          AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --target-os=mingw32"
        fi
      elif test "$compiler" = "cross-macos"; then
        AV_CONFIGURE_OPTIONS="--disable-iconv --enable-cross-compile --cross-prefix=${cross_host}- --arch=x86 --target-os=darwin --cc=$CC --cxx=$CXX"
      elif test "$compiler" = "gcc-debug" || test "$compiler" = "gcc-self-contained"; then
        test -n "$CC" && AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --cc=$CC"
        test -n "$CXX" && AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --cxx=$CXX"
      fi
      if ( test $kernel = "Darwin" || test $kernel = "MINGW" ) && test -n "${ffmpeg_version}"; then
        AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --disable-iconv"
      fi
      AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS $AV_BUILD_OPTION"
      ./configure \
	      --disable-shared \
	      --enable-static \
	      --disable-avdevice \
	      --disable-avfilter \
	      --disable-pthreads \
	      --disable-swscale \
	      --disable-network \
	      --disable-muxers \
	      --disable-demuxers \
	      --disable-sse \
	      --disable-doc \
	      --enable-rdft \
	      --enable-demuxer=aac \
	      --enable-demuxer=ac3 \
	      --enable-demuxer=ape \
	      --enable-demuxer=asf \
	      --enable-demuxer=flac \
	      --enable-demuxer=matroska_audio \
	      --enable-demuxer=mp3 \
	      --enable-demuxer=mpc \
	      --enable-demuxer=mov \
	      --enable-demuxer=mpc8 \
	      --enable-demuxer=ogg \
	      --enable-demuxer=tta \
	      --enable-demuxer=wav \
	      --enable-demuxer=wv \
	      --disable-bsfs \
	      --disable-filters \
	      --disable-parsers \
	      --enable-parser=aac \
	      --enable-parser=ac3 \
	      --enable-parser=mpegaudio \
	      --disable-protocols \
	      --enable-protocol=file \
	      --disable-indevs \
	      --disable-outdevs \
	      --disable-encoders \
	      --disable-decoders \
	      --enable-decoder=aac \
	      --enable-decoder=ac3 \
	      --enable-decoder=alac \
	      --enable-decoder=ape \
	      --enable-decoder=flac \
	      --enable-decoder=mp1 \
	      --enable-decoder=mp2 \
	      --enable-decoder=mp3 \
	      --enable-decoder=mpc7 \
	      --enable-decoder=mpc8 \
	      --enable-decoder=tta \
	      --enable-decoder=vorbis \
	      --enable-decoder=wavpack \
	      --enable-decoder=wmav1 \
	      --enable-decoder=wmav2 \
	      --enable-decoder=pcm_alaw \
	      --enable-decoder=pcm_dvd \
	      --enable-decoder=pcm_f32be \
	      --enable-decoder=pcm_f32le \
	      --enable-decoder=pcm_f64be \
	      --enable-decoder=pcm_f64le \
	      --enable-decoder=pcm_s16be \
	      --enable-decoder=pcm_s16le \
	      --enable-decoder=pcm_s16le_planar \
	      --enable-decoder=pcm_s24be \
	      --enable-decoder=pcm_daud \
	      --enable-decoder=pcm_s24le \
	      --enable-decoder=pcm_s32be \
	      --enable-decoder=pcm_s32le \
	      --enable-decoder=pcm_s8 \
	      --enable-decoder=pcm_u16be \
	      --enable-decoder=pcm_u16le \
	      --enable-decoder=pcm_u24be \
	      --enable-decoder=pcm_u24le \
	      --enable-decoder=rawvideo \
	      --disable-videotoolbox \
	      --disable-vaapi \
	      --disable-vdpau \
	      --disable-hwaccel=h264_dxva2 \
	      --disable-hwaccel=mpeg2_dxva2 $AV_CONFIGURE_OPTIONS
      make V=1
      mkdir -p inst
      make install DESTDIR=`pwd`/inst
      cd inst
      tar czf ../../bin/${ffmpeg_dir}.tgz usr
      cd ../..
      tar xmzf bin/${ffmpeg_dir}.tgz -C $BUILDROOT
    fi
  fi

  if test ! -d chromaprint-${chromaprint_version}/inst; then
    echo "### Building chromaprint"

    # The zlib library path was added for MinGW-builds GCC 4.7.2.
    cd chromaprint-${chromaprint_version}/
    test -f Makefile || eval cmake -DBUILD_SHARED_LIBS=OFF $CHROMAPRINT_ZLIB_OPTION -DFFMPEG_ROOT=$thisdir/$ffmpeg_dir/inst/usr/local $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
    make VERBOSE=1
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    fixcmakeinst
    cd inst
    tar czf ../../bin/chromaprint-${chromaprint_version}.tgz usr
    cd ../..
    tar xmzf bin/chromaprint-${chromaprint_version}.tgz -C $BUILDROOT
  fi

  if test ! -d mp4v2-${mp4v2_version}/inst; then
    echo "### Building mp4v2"

    cd mp4v2-${mp4v2_version}/
    autoreconf -i
    test -f Makefile || CXXFLAGS="$CXXFLAGS -g -O2 -DMP4V2_USE_STATIC_LIB" ./configure --enable-shared=no --enable-static=yes --disable-gch $CONFIGURE_OPTIONS
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    cd inst
    tar czf ../../bin/mp4v2-${mp4v2_version}.tgz usr
    cd ../..
    tar xmzf bin/mp4v2-${mp4v2_version}.tgz -C $BUILDROOT
  fi


  if ! test -d kid3; then
    echo "### Creating kid3 build directory"

    mkdir kid3
    if test "$compiler" = "cross-mingw"; then
      cat >kid3/build.sh <<EOF
#!/bin/bash
cmake -GNinja $CMAKE_BUILD_OPTION -DCMAKE_TOOLCHAIN_FILE=$thisdir/mingw.cmake -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DCMAKE_CXX_FLAGS="-g -O2 -DMP4V2_USE_STATIC_LIB" -DDOCBOOK_XSL_DIR=${_docbook_xsl_dir} ../../kid3
EOF
    elif test "$compiler" = "cross-macos"; then
      cat >kid3/build.sh <<EOF
#!/bin/bash
test -z \${PATH##$osxprefix/*} || PATH=$osxprefix/bin:$osxprefix/SDK/MacOSX10.13.sdk/usr/bin:\$PATH
cmake -GNinja $CMAKE_BUILD_OPTION -DCMAKE_TOOLCHAIN_FILE=$thisdir/osxcross.cmake -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DCMAKE_CXX_FLAGS="-g -O2 -DMP4V2_USE_STATIC_LIB" -DDOCBOOK_XSL_DIR=${_docbook_xsl_dir} ../../kid3
EOF
    elif test "$compiler" = "gcc-self-contained"; then
      if test -n "$QTPREFIX"; then
        _qt_prefix=$QTPREFIX
      else
        for d in /opt/qt5/${qt_version}/gcc_64 /opt/qt5/Qt${qt_version}/${qt_version}/gcc_64 $thisdir/Qt*-linux/${qt_version}/gcc_64; do
          if test -d $d; then
            _qt_prefix=$d
            break
          fi
        done
      fi
      taglib_config_version=$taglib_version
      taglib_config_version=${taglib_config_version%beta*}
      cat >kid3/build.sh <<EOF
#!/bin/bash
BUILDPREFIX=\$(cd ..; pwd)/buildroot/usr/local
export PKG_CONFIG_PATH=\$BUILDPREFIX/lib/pkgconfig
cmake -GNinja -DCMAKE_CXX_COMPILER=g++-4.8 -DCMAKE_C_COMPILER=gcc-4.8 -DQT_QMAKE_EXECUTABLE=${_qt_prefix}/bin/qmake -DWITH_READLINE=OFF -DBUILD_SHARED_LIBS=ON -DLINUX_SELF_CONTAINED=ON -DWITH_TAGLIB=OFF -DHAVE_TAGLIB=1 -DTAGLIB_LIBRARIES:STRING="-L\$BUILDPREFIX/lib -ltag -lz" -DTAGLIB_CFLAGS:STRING="-I\$BUILDPREFIX/include/taglib -I\$BUILDPREFIX/include -DTAGLIB_STATIC" -DTAGLIB_VERSION:STRING="${taglib_config_version}" -DWITH_QML=ON -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL" -DCMAKE_INCLUDE_PATH=\$BUILDPREFIX/include -DCMAKE_LIBRARY_PATH=\$BUILDPREFIX/lib -DCMAKE_PROGRAM_PATH=\$BUILDPREFIX/bin -DWITH_FFMPEG=ON -DFFMPEG_ROOT=\$BUILDPREFIX -DWITH_MP4V2=ON $CMAKE_BUILD_OPTION -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. -DWITH_LIBDIR=. -DWITH_PLUGINSDIR=./plugins ../../kid3
EOF
    elif test $kernel = "Darwin"; then
      _qt_prefix=${QTPREFIX:-/usr/local/Trolltech/Qt${qt_version}/${qt_version}/clang_64}
      cat >kid3/build.sh <<EOF
#!/bin/bash
INCLUDE=../buildroot/usr/local/include LIB=../buildroot/usr/local/lib cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQT_QMAKE_EXECUTABLE=${_qt_prefix}/bin/qmake -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DWITH_DOCBOOKDIR=${_docbook_xsl_dir} ../../kid3
EOF
    elif test $kernel = "MINGW"; then
      _qtToolsMingw=($QTPREFIX/../../Tools/mingw*)
      _qtToolsMingw=$(realpath $_qtToolsMingw)
      cat >kid3/build.sh <<EOF
#!/bin/bash
INCLUDE=../buildroot/usr/local/include LIB=../buildroot/usr/local/lib cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQT_QMAKE_EXECUTABLE=${QTPREFIX}/bin/qmake -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DWITH_DOCBOOKDIR=${_docbook_xsl_dir:-$HOME/prg/docbook-xsl-1.72.0} ../../kid3
EOF
      _qtPrefixWin=${QTPREFIX//\//\\}
      _qtPrefixWin=${_qtPrefixWin/\\c/C:}
      _qtToolsMingwWin=${_qtToolsMingw//\//\\}
      _qtToolsMingwWin=${_qtToolsMingwWin/\\c/C:}
      _docbookXslDirWin=${_docbook_xsl_dir//\//\\}
      _docbookXslDirWin=${_docbookXslDirWin/\\c/C:}
      cat >kid3/build.bat <<EOF
set INCLUDE=../buildroot/usr/local/include
set LIB=../buildroot/usr/local/lib
echo ;%PATH%; | find /C /I ";$_qtPrefixWin\bin;"
if errorlevel 1 (
  path $_qtPrefixWin\bin;$_qtToolsMingwWin\bin;$_qtToolsMingwWin\opt\bin;C:\Python38;%PATH%
)
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DWITH_DOCBOOKDIR=${_docbookXslDirWin:-%HOME%/prg/docbook-xsl-1.72.0} ../../kid3
EOF
      cat >kid3/run.bat <<EOF
set thisdir=%~dp0
echo ;%PATH%; | find /C /I ";$_qtPrefixWin\bin;"
if errorlevel 1 (
  path $_qtPrefixWin\bin;$_qtToolsMingwWin\bin;$_qtToolsMingwWin\opt\bin;C:\Python38;%HOME%\prg\dumpbin;%PATH%
)
echo ;%PATH%; | find /C /I ";%thisdir%src\core;"
if errorlevel 1 (
  path %thisdir%src\core;%thisdir%src\gui;%PATH%
)
set QT_PLUGIN_PATH=$_qtPrefixWin\plugins
start src\app\qt\kid3
EOF
    elif test "$compiler" = "gcc-debug"; then
      taglib_config_version=$taglib_version
      taglib_config_version=${taglib_config_version%beta*}
      cat >kid3/build.sh <<EOF
#!/bin/bash
BUILDPREFIX=\$(cd ..; pwd)/buildroot/usr/local
export PKG_CONFIG_PATH=\$BUILDPREFIX/lib/pkgconfig
cmake -GNinja -DBUILD_SHARED_LIBS=ON -DQT_QMAKE_EXECUTABLE=${QT_PREFIX}/bin/qmake -DLINUX_SELF_CONTAINED=ON -DWITH_READLINE=OFF -DWITH_TAGLIB=OFF -DHAVE_TAGLIB=1 -DTAGLIB_LIBRARIES:STRING="-L\$BUILDPREFIX/lib -ltag -lz" -DTAGLIB_CFLAGS:STRING="-I\$BUILDPREFIX/include/taglib -I\$BUILDPREFIX/include -DTAGLIB_STATIC" -DTAGLIB_VERSION:STRING="${taglib_config_version}" -DWITH_QML=ON -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL" -DCMAKE_INCLUDE_PATH=\$BUILDPREFIX/include -DCMAKE_LIBRARY_PATH=\$BUILDPREFIX/lib -DCMAKE_PROGRAM_PATH=\$BUILDPREFIX/bin -DWITH_FFMPEG=ON -DFFMPEG_ROOT=\$BUILDPREFIX -DWITH_MP4V2=ON $CMAKE_BUILD_OPTION -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. -DWITH_LIBDIR=. -DWITH_PLUGINSDIR=./plugins ../../kid3
EOF
    else
      taglib_config_version=$taglib_version
      taglib_config_version=${taglib_config_version%beta*}
      cat >kid3/build.sh <<EOF
#!/bin/bash
BUILDPREFIX=\$(cd ..; pwd)/buildroot/usr/local
export PKG_CONFIG_PATH=\$BUILDPREFIX/lib/pkgconfig
cmake -GNinja -DBUILD_SHARED_LIBS=ON -DLINUX_SELF_CONTAINED=ON -DWITH_TAGLIB=OFF -DHAVE_TAGLIB=1 -DTAGLIB_LIBRARIES:STRING="-L\$BUILDPREFIX/lib -ltag -lz" -DTAGLIB_CFLAGS:STRING="-I\$BUILDPREFIX/include/taglib -I\$BUILDPREFIX/include -DTAGLIB_STATIC" -DTAGLIB_VERSION:STRING="${taglib_config_version}" -DWITH_QML=ON -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL" -DCMAKE_INCLUDE_PATH=\$BUILDPREFIX/include -DCMAKE_LIBRARY_PATH=\$BUILDPREFIX/lib -DCMAKE_PROGRAM_PATH=\$BUILDPREFIX/bin -DWITH_FFMPEG=ON -DFFMPEG_ROOT=\$BUILDPREFIX -DWITH_MP4V2=ON $CMAKE_BUILD_OPTION -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. -DWITH_LIBDIR=. -DWITH_PLUGINSDIR=./plugins ../../kid3
EOF
    fi
    chmod +x kid3/build.sh
  fi

fi # cross-android, msvc, else
fi # libs

if [[ $target = *"package"* ]]; then
  echo "### Building kid3 package"

  pushd kid3 >/dev/null
  if test -f build.sh && ! test -f Makefile && ! test -f build.ninja; then
    ./build.sh
  fi
  if test "$compiler" = "cross-mingw"; then
    ninja
    _version=$(grep VERSION config.h | cut -d'"' -f2)
    if test -z "${cross_host##x86_64*}"; then
      _gccDll=libgcc_s_seh-1.dll
      _instdir=kid3-$_version-win32-x64
    else
      _gccDll=libgcc_s_dw2-1.dll
      _instdir=kid3-$_version-win32
    fi
    test -d $_instdir && rm -rf $_instdir
    mkdir -p $_instdir
    DESTDIR=$(pwd)/$_instdir ninja install/strip

    _plugin_qt_version=$(grep "Created by.*Qt" src/plugins/musicbrainzimport/moc_musicbrainzimportplugin.cpp)
    _plugin_qt_version=${_plugin_qt_version##* \(Qt }
    _plugin_qt_version=${_plugin_qt_version%%\)*}
    _plugin_qt_version_nr=${_plugin_qt_version//./}
    if test $_plugin_qt_version_nr -gt ${qt_version//./}; then
      echo "Plugin Qt version $_plugin_qt_version is larger than Qt version $qt_version."
      echo "Loading plugins will fail!"
      exit 1
    fi

    cp -f translations/*.qm doc/*/kid3*.html $_instdir

    _qtBinDir=${QTPREFIX}/bin
    for f in Qt5Core.dll Qt5Network.dll Qt5Gui.dll Qt5Xml.dll Qt5Widgets.dll Qt5Multimedia.dll Qt5Qml.dll Qt5Quick.dll $_gccDll libstdc++-6.dll libwinpthread-1.dll; do
      cp $_qtBinDir/$f $_instdir
    done

    _qtTranslationsDir=${QTPREFIX}/translations
    for f in translations/*.qm; do
      l=${f#*_};
      l=${l%.qm};
      test -f $_qtTranslationsDir/qtbase_$l.qm && cp $_qtTranslationsDir/qtbase_$l.qm $_instdir
    done

    rm -f $_instdir.zip
    7z a $_instdir.zip $_instdir
  elif test "$compiler" = "cross-macos"; then
    test -z ${PATH##$osxprefix/*} || PATH=$osxprefix/bin:$osxprefix/SDK/MacOSX10.13.sdk/usr/bin:$PATH
    rm -rf inst
    DESTDIR=$(pwd)/inst ninja install/strip
    ln -s /Applications inst/Applications
    genisoimage -V "Kid3" -D -R -apple -no-pad -o uncompressed.dmg inst
    _version=$(grep VERSION config.h | cut -d'"' -f2)
    dmg dmg uncompressed.dmg kid3-$_version-Darwin.dmg
    rm uncompressed.dmg
  elif test "$compiler" = "cross-android"; then
    JAVA_HOME=$(grep _java_root= build.sh | cut -d'=' -f2) make apk
    _version=$(grep VERSION config.h | cut -d'"' -f2)
    for prefix in android/build/outputs/apk/release/android-release android/build/outputs/apk/android-release android/bin/QtApp-release; do
      for suffix in signed unsigned; do
        _apkpath=${prefix}-${suffix}.apk
        if test -f $_apkpath; then
          cp -a $_apkpath kid3-$_version-android.apk
          break 2
        fi
      done
    done
  elif test "$compiler" = "gcc-self-contained"; then
    ninja package
    _tgz=(kid3-*-Linux.tar.gz)
    test -f "$_tgz" && mv $_tgz ${_tgz%%tar.gz}tgz
  elif test "$compiler" = "gcc-debug"; then
    ninja
  else
    if test -f build.ninja; then
      ninja package
    else
      make package
    fi
  fi
  popd >/dev/null
fi # package

echo "### Built successfully"
