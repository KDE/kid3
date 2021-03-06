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
  sed -i "s/^Version:        ${OLDVER}$/Version:        ${NEWVER}/" kid3.spec
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
  sed -i -r "s/(for ver in \[[^]]*'${OLDVER}')\]:/\1, '${NEWVER}']:/" packaging/craft/extragear/kid3/kid3.py
  sed -i "s,https://download.kde.org/stable/kid3/${OLDVER}/kid3-${OLDVER}.tar.xz,https://download.kde.org/stable/kid3/${NEWVER}/kid3-${NEWVER}.tar.xz," packaging/flatpak/org.kde.kid3-stable.json
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
  if hash podman 2>/dev/null; then
    DOCKER=podman
  else
    DOCKER=docker
  fi
  $DOCKER build -t ufleisch/kid3dev . -f-<<EOF
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
  if hash podman 2>/dev/null; then
    DOCKER=podman
    USERNSARG=--userns=keep-id
  else
    DOCKER=docker
    USERNSARG=
  fi
  $DOCKER run $USERNSARG --rm -it -e LANG=C.UTF-8 \
         -v $HOME/projects/kid3:$HOME/projects/kid3 \
         -v $HOME/.gradle:$HOME/.gradle \
         -v $HOME/.gnupg:$HOME/.gnupg:ro \
         -v $HOME/Development:$HOME/Development:ro ufleisch/kid3dev "$@"
  exit 0
fi

# Build flatpak
if test "$1" = "flatpak"; then
  echo "### Build flatpak"
  flatpak-builder --sandbox --force-clean --ccache --repo=repo --subject="Build of org.kde.kid3 $(date --iso-8601=seconds)" app "$srcdir/packaging/flatpak/org.kde.kid3-local.json"
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
ffmpeg3_version=3.2.14
ffmpeg3_patchlevel=1~deb9u1
ffmpeg_version=4.1.6
ffmpeg_patchlevel=1~deb10u1
libflac_version=1.3.3
libflac_patchlevel=2
id3lib_version=3.8.3
id3lib_patchlevel=16.3
taglib_version=1.12
chromaprint_version=1.5.0
chromaprint_patchlevel=2
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

if [[ "$QTPREFIX" =~ /([0-9]+)\.([0-9]+)\.([0-9]+)/ ]]; then
  qt_nr=$(printf "%d%02d%02d" ${BASH_REMATCH[1]} ${BASH_REMATCH[2]} ${BASH_REMATCH[3]})
else
  echo "Could not extract Qt version from $QTPREFIX"
  exit 1
fi

if test "$qt_nr" -ge 51204; then
  # Since Qt 5.12.4, OpenSSL 1.1.1 is supported
  openssl_version=1.1.1g
else
  openssl_version=1.0.2u
fi

if test "$compiler" = "gcc-self-contained"; then
  if test "$qt_nr" -lt 60000; then
    gcc_self_contained_cc="gcc-4.8"
    gcc_self_contained_cxx="g++-4.8"
  else
    gcc_self_contained_cc="gcc"
    gcc_self_contained_cxx="g++"
  fi
fi

if test "$compiler" = "cross-mingw"; then
  if test -n "$QTPREFIX" && test -z "${QTPREFIX%%*64?(/)}"; then
    cross_host="x86_64-w64-mingw32"
  else
    cross_host="i686-w64-mingw32"
    # FFmpeg > 3 is not compatible with Windows XP
    ffmpeg_version=$ffmpeg3_version
    ffmpeg_patchlevel=$ffmpeg3_patchlevel
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
  export CC=$gcc_self_contained_cc
  export CXX=$gcc_self_contained_cxx
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
  # Qt uses posix threads, but x86_64-w64-mingw32-gcc uses win32 thread model.
  if test "$cross_host" = "x86_64-w64-mingw32"; then
    export CC=${cross_host}-gcc-posix
    export CXX=${cross_host}-g++-posix
  fi
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
  if [[ $(sw_vers -productVersion) = 10.1* || $(sw_vers -productVersion) = 11.* ]]; then
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

if test -n "${taglib_version##v*}"; then
  test -f taglib-${taglib_version}.tar.gz ||
    $DOWNLOAD http://taglib.github.io/releases/taglib-${taglib_version}.tar.gz
else
  # Download an archive for a git tag
  if ! test -f taglib-${taglib_version##v}.tar.gz; then
    $DOWNLOAD https://github.com/taglib/taglib/archive/${taglib_version}.tar.gz
    mv ${taglib_version}.tar.gz taglib-${taglib_version##v}.tar.gz
  fi
  taglib_version=${taglib_version##v}
fi

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

  test -f ffmpeg_${ffmpeg_version}.orig.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/ffmpeg/ffmpeg_${ffmpeg_version}.orig.tar.xz
  test -f ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/ffmpeg/ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz

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
  if test "$cross_host" = "x86_64-w64-mingw32"; then
    _thread_suffix=-posix
  else
    _thread_suffix=
  fi
  cat >$thisdir/mingw.cmake <<EOF
set(QT_PREFIX ${_qt_prefix})

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER ${cross_host}-gcc${_thread_suffix})
set(CMAKE_CXX_COMPILER ${cross_host}-g++${_thread_suffix})
set(CMAKE_RC_COMPILER ${cross_host}-windres)
set(CMAKE_FIND_ROOT_PATH /usr/${cross_host} \${QT_PREFIX} $thisdir/buildroot/usr/local ${ZLIB_ROOT_PATH} $thisdir/ffmpeg-${ffmpeg_version}/inst/usr/local)
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
  if test "$taglib_nr" = "112"; then
    if test "$cross_host" = "x86_64-w64-mingw32"; then
      patch -p1 <$srcdir/packaging/patches/taglib-1.12-win00-large_file.patch
    fi
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
    patch -p1 <$srcdir/packaging/patches/flac-1.2.1-00-size_t_max.patch
    if test $kernel = "Darwin"; then
      patch -p1 <$srcdir/packaging/patches/flac-1.2.1-mac00-fink.patch
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
    patch -p1 <$srcdir/packaging/patches/id3lib-3.8.3-win00-mingw.patch
    patch -p1 <$srcdir/packaging/patches/id3lib-3.8.3-win01-tempfile.patch
    test -f makefile.win32.orig || mv makefile.win32 makefile.win32.orig
    sed 's/-W3 -WX -GX/-W3 -EHsc/; s/-MD -D "WIN32" -D "_DEBUG"/-MDd -D "WIN32" -D "_DEBUG"/' makefile.win32.orig >makefile.win32
    cd ..
  fi

  if ! test -d ffmpeg-${ffmpeg_version}; then
    echo "### Extracting ffmpeg"

    tar xJf source/ffmpeg_${ffmpeg_version}.orig.tar.xz || true
    cd ffmpeg-${ffmpeg_version}/
    tar xJf ../source/ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz || true
    for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    cd ..
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
      patch -p1 <$srcdir/packaging/patches/mp4v2-1.0.0-win00-platform.patch
      if test -z "${cross_host##x86_64*}"; then
        sed -i '/^#   define _USE_32BIT_TIME_T/ s#^#//#' libplatform/platform_win32.h
      fi
    fi
    patch -p1 <$srcdir/packaging/patches/mp4v2-1.0.0-0005-Cxx11_compiler.patch
    patch -p1 <$srcdir/packaging/patches/mp4v2-1.0.0-0004-Pointer_comparison.patch
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
    patch -p0 <$srcdir/packaging/patches/openssl-1.1.1-android00-setenv.patch
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
    cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$BUILDROOT/usr/local -DANDROID_NDK=$_android_ndk_root -DANDROID_ABI=$_android_abi -DCMAKE_TOOLCHAIN_FILE=$_android_toolchain_cmake -DANDROID_PLATFORM=$_android_platform -DANDROID_CCACHE=$_android_ccache -DCMAKE_MAKE_PROGRAM=make
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

else #  cross-android

  if test "$1" = "clean"; then
    for d in zlib-${zlib_version} libogg-${libogg_version} \
             libvorbis-${libvorbis_version} flac-${libflac_version} \
             id3lib-${id3lib_version} taglib-${taglib_version} \
             ffmpeg-${ffmpeg_version} chromaprint-${chromaprint_version} \
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
    if test "$cross_host" = "x86_64-w64-mingw32"; then
      _cctmp=$CC
      CC=gcc-posix
    fi
    ./Configure shared enable-ec_nistp_64_gcc_128 $_target --cross-compile-prefix=$_crossprefix
    make depend || true
    make build_libs
    if test "$cross_host" = "x86_64-w64-mingw32"; then
      CC=$_cctmp
    fi
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
    test -f Makefile || eval cmake -DINCLUDE_DIRECTORIES=/usr/local/include -DLINK_DIRECTORIES=/usr/local/lib -DBUILD_SHARED_LIBS=OFF $TAGLIB_ZLIB_ROOT_OPTION $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
    make VERBOSE=1
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    fixcmakeinst
    cd inst
    tar czf ../../bin/taglib-${taglib_version}.tgz usr
    cd ../..
    tar xmzf bin/taglib-${taglib_version}.tgz -C $BUILDROOT
  fi

  if test ! -d ffmpeg-${ffmpeg_version}/inst; then
    echo "### Building ffmpeg"

    cd ffmpeg-${ffmpeg_version}
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
      else
        AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --extra-ldflags=-lbcrypt"
      fi
      test -n "$CC" && AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --cc=$CC"
      test -n "$CXX" && AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --cxx=$CXX"
    elif test $kernel = "MINGW"; then
      # mkstemp is not available when building with mingw from Qt
      sed -i 's/check_func  mkstemp/disable  mkstemp/' ./configure
      if ! [[ $(uname) =~ ^MINGW64 ]]; then
        AV_CONFIGURE_OPTIONS="--extra-cflags=-march=i486"
      else
        AV_CONFIGURE_OPTIONS="--extra-ldflags=-lbcrypt"
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
    if test $kernel = "Darwin" || test $kernel = "MINGW"; then
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
    tar czf ../../bin/ffmpeg-${ffmpeg_version}.tgz usr
    cd ../..
    tar xmzf bin/ffmpeg-${ffmpeg_version}.tgz -C $BUILDROOT
  fi

  if test ! -d chromaprint-${chromaprint_version}/inst; then
    echo "### Building chromaprint"

    # The zlib library path was added for MinGW-builds GCC 4.7.2.
    cd chromaprint-${chromaprint_version}/
    test -f Makefile || eval cmake -DBUILD_SHARED_LIBS=OFF $CHROMAPRINT_ZLIB_OPTION -DFFMPEG_ROOT=$thisdir/ffmpeg-${ffmpeg_version}/inst/usr/local $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
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
cmake -GNinja -DCMAKE_CXX_COMPILER=${gcc_self_contained_cxx} -DCMAKE_C_COMPILER=${gcc_self_contained_cc} -DQT_QMAKE_EXECUTABLE=${_qt_prefix}/bin/qmake -DWITH_READLINE=OFF -DBUILD_SHARED_LIBS=ON -DLINUX_SELF_CONTAINED=ON -DWITH_TAGLIB=OFF -DHAVE_TAGLIB=1 -DTAGLIB_LIBRARIES:STRING="-L\$BUILDPREFIX/lib -ltag -lz" -DTAGLIB_CFLAGS:STRING="-I\$BUILDPREFIX/include/taglib -I\$BUILDPREFIX/include -DTAGLIB_STATIC" -DTAGLIB_VERSION:STRING="${taglib_config_version}" -DWITH_QML=ON -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL" -DCMAKE_INCLUDE_PATH=\$BUILDPREFIX/include -DCMAKE_LIBRARY_PATH=\$BUILDPREFIX/lib -DCMAKE_PROGRAM_PATH=\$BUILDPREFIX/bin -DWITH_FFMPEG=ON -DFFMPEG_ROOT=\$BUILDPREFIX -DWITH_MP4V2=ON $CMAKE_BUILD_OPTION -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. -DWITH_LIBDIR=. -DWITH_PLUGINSDIR=./plugins ../../kid3
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

fi # cross-android, else
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
