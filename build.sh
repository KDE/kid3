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
# pacman -S git patch autoconf automake make docbook-xsl mingw-w64-x86_64-make \
#   mingw-w64-x86_64-ninja mingw-w64-x86_64-nasm mingw-w64-x86_64-libtool
# Start the msys shell, add Qt and cmake to the path and start this script.
#
# export QTPREFIX=/c/Qt/6.5.3/mingw_64
# test -z "${PATH##$QTPREFIX*}" ||
# PATH=$QTPREFIX/bin:$QTPREFIX/../../Tools/mingw1120_64/bin:$QTPREFIX/../../Tools/mingw1120_64/opt/bin:$PROGRAMFILES/CMake/bin:$PATH
# ../kid3/build.sh
#
# You can also build a Windows version from Linux using the MinGW cross
# compiler.
# COMPILER=cross-mingw QTPREFIX=/path/to/Qt6.5.3-mingw64/6.5.3/mingw_64 \
#   QTBINARYDIR=/path/to/Qt6.5.3-linux/6.5.3/gcc_64/bin ../kid3/build.sh
#
# For Mac:
#
# The build dependencies can be installed with Homebrew, for instance:
# brew install cmake ninja autoconf automake libtool xz nasm docbook-xsl qt
# Then call from a build directory
# ../kid3/build.sh
#
# You can also build a macOS version from Linux using the osxcross toolchain.
# COMPILER=cross-macos QTPREFIX=/path/to/Qt6.5.3-mac/6.5.3/macos ../kid3/build.sh
# or
# COMPILER=cross-macos QTPREFIX=/path/to/Qt6.5.3-mac/6.5.3/macos \
#   QTBINARYDIR=/path/to/Qt6.5.3-linux/6.5.3/gcc_64/bin ../kid3/build.sh
#
# For Android:
#
# Install Qt and a compatible Android SDK and NDK, for example Qt 5.9.7,
# NDK 10e or Qt 5.12.2, NDK 19c, or Qt 6.5.3, NDK 23.1.7779620.
# COMPILER=cross-android QTPREFIX=/path/to/Qt/6.5.3/android_armv7 \
#   QTBINARYDIR=/path/to/Qt6.5.3-linux/6.5.3/gcc_64/bin \
#   ANDROID_SDK_ROOT=/path/to/sdk ANDROID_NDK_ROOT=/path/to/ndk/23.1.7779620 \
#   ../kid3/build.sh
#
# For Linux:
#
# To build a self-contained Linux package use
# COMPILER=gcc-self-contained QTPREFIX=/path/to/Qt6.5.3-linux/6.5.3/gcc_64 \
#   ../kid3/build.sh
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
# build.sh will download, build and install zlib, libogg, libvorbis,
# flac, id3lib, taglib, ffmpeg, chromaprint, mp4v2. When the libraries
# are built, the Kid3 package is built. It is also possible to build only
# the libraries or only the Kid3 package.
#
# ../kid3/build.sh libs
# ../kid3/build.sh package

# Exit if an error occurs
set -eE
trap 'echo "Error on line $LINENO: $BASH_COMMAND"' ERR
shopt -s extglob

thisdir=$(pwd)
srcdir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

kernel=$(uname)
test ${kernel:0:5} = "MINGW" && kernel="MINGW"

qt_version=6.5.3
zlib_version=1.2.13
zlib_patchlevel=1
libogg_version=1.3.4
libogg_patchlevel=0.1
libvorbis_version=1.3.7
libvorbis_patchlevel=2
ffmpeg3_version=3.2.14
ffmpeg3_patchlevel=1~deb9u1
ffmpeg_version=5.1.6
ffmpeg_patchlevel=0+deb12u1
libflac_version=1.4.3+ds
libflac_patchlevel=2.1
id3lib_version=3.8.3
id3lib_patchlevel=18
taglib_version=2.0.2
chromaprint_version=1.5.1
chromaprint_patchlevel=6
mp4v2_version=2.1.3
utfcpp_version=4.0.5

verify_not_in_srcdir() {
  if test -f CMakeLists.txt; then
    echo "Do not run this script from the source directory!"
    echo "Start it from a build directory at the same level as the source directory."
    exit 1
  fi
}

verify_in_srcdir() {
  if ! test -f CMakeLists.txt; then
    echo "Run this task from the source directory!"
    exit 1
  fi
}

download_and_extract_qt() {
  if ! test -d qt-${qt_version}; then
    mkdir qt-${qt_version}
    cd qt-${qt_version}
    if test $qt_version = "6.5.3"; then
      if test "$compiler" = "cross-android"; then
        if test "$_android_abi" = "x86_64"; then
          for m in qttranslations qttools qtsvg qtdeclarative qtbase; do
            fn=6.5.3-0-202309260341${m}-Linux-RHEL_8_4-Clang-Android-Android_ANY-X86_64.7z
            wget https://download.qt.io/online/qtsdkrepository/linux_x64/android/qt6_653_x86_64/qt.qt6.653.android_x86_64/$fn
          done
          for m in qtmultimedia qtimageformats; do
            fn=qt.qt6.653.addons.${m}.android_x86_64/6.5.3-0-202309260341${m}-Linux-RHEL_8_4-Clang-Android-Android_ANY-X86_64.7z
            wget https://download.qt.io/online/qtsdkrepository/linux_x64/android/qt6_653_x86_64/$fn
          done
        elif test "$_android_abi" = "arm64-v8a"; then
          for m in qttranslations qttools qtsvg qtdeclarative qtbase; do
            fn=6.5.3-0-202309260341${m}-MacOS-MacOS_12-Clang-Android-Android_ANY-ARM64.7z
            wget https://download.qt.io/online/qtsdkrepository/linux_x64/android/qt6_653_arm64_v8a/qt.qt6.653.android_arm64_v8a/$fn
          done
          for m in qtmultimedia qtimageformats; do
            fn=qt.qt6.653.addons.${m}.android_arm64_v8a/6.5.3-0-202309260341${m}-MacOS-MacOS_12-Clang-Android-Android_ANY-ARM64.7z
            wget https://download.qt.io/online/qtsdkrepository/linux_x64/android/qt6_653_arm64_v8a/$fn
          done
        else
          for m in qttranslations qttools qtsvg qtdeclarative qtbase; do
            fn=6.5.3-0-202309260341${m}-Windows-Windows_10_22H2-Clang-Android-Android_ANY-ARMv7.7z
            $DOWNLOAD https://download.qt.io/online/qtsdkrepository/linux_x64/android/qt6_653_armv7/qt.qt6.653.android_armv7/$fn
          done
          for m in qtmultimedia qtimageformats; do
            fn=qt.qt6.653.addons.${m}.android_armv7/6.5.3-0-202309260341${m}-Windows-Windows_10_22H2-Clang-Android-Android_ANY-ARMv7.7z
            $DOWNLOAD https://download.qt.io/online/qtsdkrepository/linux_x64/android/qt6_653_armv7/$fn
          done
        fi
      elif test $kernel = "Linux"; then
        for m in qttranslations qttools qtsvg qtdeclarative qtbase qtwayland; do
          fn=6.5.3-0-202309260341${m}-Linux-RHEL_8_4-GCC-Linux-RHEL_8_4-X86_64.7z
          $DOWNLOAD https://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt6_653/qt.qt6.653.gcc_64/$fn
        done
        $DOWNLOAD https://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt6_653/qt.qt6.653.gcc_64/6.5.3-0-202309260341icu-linux-Rhel7.2-x64.7z
        for m in qtmultimedia qtimageformats; do
          fn=qt.qt6.653.addons.${m}.gcc_64/6.5.3-0-202309260341${m}-Linux-RHEL_8_4-GCC-Linux-RHEL_8_4-X86_64.7z
          $DOWNLOAD https://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt6_653/$fn
        done
      elif test $kernel = "Darwin"; then
        for m in qttranslations qttools qtsvg qtdeclarative qtbase; do
          fn=6.5.3-0-202309260341${m}-MacOS-MacOS_12-Clang-MacOS-MacOS_12-X86_64-ARM64.7z
          $DOWNLOAD https://download.qt.io/online/qtsdkrepository/mac_x64/desktop/qt6_653/qt.qt6.653.clang_64/$fn
        done
        for m in qtmultimedia qtimageformats; do
          fn=qt.qt6.653.addons.${m}.clang_64/6.5.3-0-202309260341${m}-MacOS-MacOS_12-Clang-MacOS-MacOS_12-X86_64-ARM64.7z
          $DOWNLOAD https://download.qt.io/online/qtsdkrepository/mac_x64/desktop/qt6_653/$fn
        done
      elif test $kernel = "MINGW"; then
        for m in qttranslations qttools qtsvg qtdeclarative qtbase; do
          fn=6.5.3-0-202309260341${m}-Windows-Windows_10_22H2-Mingw-Windows-Windows_10_22H2-X86_64.7z
          $DOWNLOAD https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_653/qt.qt6.653.win64_mingw/$fn
        done
        for m in opengl32sw-64-mesa_11_2_2-signed_sha256 d3dcompiler_47-x64 MinGW-w64-x86_64-11.2.0-release-posix-seh-rt_v9-rev1-runtime; do
          fn=6.5.3-0-202309260341${m}.7z
          $DOWNLOAD https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_653/qt.qt6.653.win64_mingw/$fn
        done
        for m in qtmultimedia qtimageformats; do
          fn=qt.qt6.653.addons.${m}.win64_mingw/6.5.3-0-202309260341${m}-Windows-Windows_10_22H2-Mingw-Windows-Windows_10_22H2-X86_64.7z
          $DOWNLOAD https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_653/$fn
        done
        for m in mingw-w64-x86_64-11.2.0-release-posix-seh-rt_v9-rev3; do
          fn=9.0.0-1-202203221220${m}.7z
          $DOWNLOAD https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/tools_mingw90/qt.tools.win64_mingw900/$fn
        done
      fi
      for fn in *.7z; do 7za x $fn; done
      rm -f *.7z
    fi
    cd -
  fi
}

# The environment variable KID3_HOMEBREW_PKGS can be set to install packages
# needed to build using Homebrew, if brew is not yet available a local instance
# is installed.
# Example: KID3_HOMEBREW_PKGS="cmake ninja autoconf automake libtool xz nasm docbook-xsl p7zip"
if test -n "$KID3_HOMEBREW_PKGS"; then
  export HOMEBREW_NO_INSTALL_UPGRADE=1
  if ! brew install $KID3_HOMEBREW_PKGS; then
    mkdir -p tools/Homebrew
    curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C tools/Homebrew
    eval "$(tools/Homebrew/bin/brew shellenv)"
    brew install $KID3_HOMEBREW_PKGS
  fi
fi

# Administrative subtasks

# Changes version and date strings in all known Kid3 files.
if test "$1" = "changeversion"; then
  OLDVER=$2
  NEWVER=$3
  if test -z "$OLDVER" || test -z "$NEWVER"; then
    echo "Usage: $0 $1 old-version-nr new-version-nr [--finalize], e.g. $0 $1 0.8 0.9"
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
  grep -qF "kid3 (${NEWVER}-0)" deb/changelog ||
  sed -i "1 i\
kid3 (${NEWVER}-0) unstable; urgency=low\n\n  * New upstream release.\n\n\
 -- Urs Fleisch <ufleisch@users.sourceforge.net>  ${DATE_R}\n" deb/changelog
  grep -qF "<releaseinfo>${NEWVER}</releaseinfo>" doc/en/index.docbook ||
  sed -i "s/^<releaseinfo>${OLDVER}<\/releaseinfo>$/<releaseinfo>${NEWVER}<\/releaseinfo>/; s/^<year>[0-9]\+<\/year>$/<year>${DATE_Y}<\/year>/; s/^<date>[0-9-]\+<\/date>$/<date>${DATE_F}<\/date>/" doc/en/index.docbook
  sed -i "s/PROJECTVERSION=\"${OLDVER}\"/PROJECTVERSION=\"${NEWVER}\"/" translations/extract-merge.sh
  # kid3.spec is used by f-droid to detect a new version, it should be updated exactly before the release.
  if test "$4" = "--finalize"; then
    sed -i "s/^\(Version: \+\).*$/\1${NEWVER}/" kid3.spec
    OLDCODE=$(sed -n "s/ \+set(QT_ANDROID_APP_VERSION_CODE \([0-9]\+\))/\1/p" CMakeLists.txt)
    NEWCODE=$[ $OLDCODE + 1 ]
    sed -i "s/\( \+set(QT_ANDROID_APP_VERSION_CODE \)\([0-9]\+\))/\1$NEWCODE)/" CMakeLists.txt
  fi
  sed -i "s/^Copyright 2003-[0-9]\+ Urs Fleisch <ufleisch@users.sourceforge.net>$/Copyright 2003-${DATE_Y} Urs Fleisch <ufleisch@users.sourceforge.net>/" deb/copyright
  grep -qF "* Release ${NEWVER}" ChangeLog ||
  sed -i "1 i\
${DATE}  Urs Fleisch  <ufleisch@users.sourceforge.net>\n\n\t* Release ${NEWVER}\n" ChangeLog
  sed -i "s/PROJECT_NUMBER         = ${OLDVER}/PROJECT_NUMBER         = ${NEWVER}/" Doxyfile
  sed -i "s/set(CPACK_PACKAGE_VERSION_MAJOR ${OLDMAJOR})/set(CPACK_PACKAGE_VERSION_MAJOR ${NEWMAJOR})/; s/set(CPACK_PACKAGE_VERSION_MINOR ${OLDMINOR})/set(CPACK_PACKAGE_VERSION_MINOR ${NEWMINOR})/; s/set(CPACK_PACKAGE_VERSION_PATCH ${OLDPATCH})/set(CPACK_PACKAGE_VERSION_PATCH ${NEWPATCH})/; s/set(RELEASE_YEAR [0-9]\+)/set(RELEASE_YEAR ${DATE_Y})/" CMakeLists.txt
  grep -qF "<release version=\"${NEWVER}\"" src/app/org.kde.kid3.appdata.xml ||
  sed -i "s,^  <releases>.*,&\n    <release version=\"${NEWVER}\" date=\"${DATE_F}\"/>," src/app/org.kde.kid3.appdata.xml
  grep -q "for ver in .*'${NEWVER}'" packaging/craft/extragear/kid3/kid3.py ||
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
# You can then build all packages using
# kid3/build.sh rundocker $HOME/projects/kid3/src/build-all.sh
# You need:
# - Kid3 project checked out in ~/projects/kid3/src/kid3
# - At least CMake 3.21 in /opt/cmake/bin/
# Linux:
# - Qt 6.5.3 Linux in ~/Development/Qt6.5.3-linux/6.5.3/gcc_64/
# Windows:
# - MinGW 11.2 cross compiler in /opt/mxe11/
# - Qt 6.5.3 MinGW64 in ~/Development/Qt6.5.3-mingw64/6.5.3/mingw_64/
# Mac:
# - Mac cross compiler in /opt/osxcross/
# - Qt 6.5.3 Mac in ~/Development/Qt6.5.3-mac/6.5.3/macos/
# Android:
# - Java JDK 17
# - Android SDK in ~/Development/android-sdk/
# - Android NDK in ~/Development/android-sdk/ndk/23.1.7779620/
# - Qt 6.5.3 Android in ~/Development/Qt6.5.3-android/6.5.3/android_armv7/
# - Sign key in ~/Development/ufleisch-release-key.keystore
# - Gradle cache in ~/.gradle/
if test "$1" = "makedocker"; then
  verify_not_in_srcdir
  if ! test -f build-all.sh; then
    cat >build-all.sh <<"EOF"
#!/bin/bash
cd "$(dirname "${BASH_SOURCE[0]}")"
set -e
(cd linux_build && \
   PATH=/opt/cmake/bin:$PATH \
   COMPILER=gcc-self-contained \
   QTPREFIX=$HOME/Development/Qt6.5.3-linux/6.5.3/gcc_64 \
   ../kid3/build.sh)
(cd mingw64_build && \
   PATH=/opt/mxe11/usr/bin:/opt/cmake/bin:$PATH \
   COMPILER=cross-mingw \
   QTPREFIX=$HOME/Development/Qt6.5.3-mingw64/6.5.3/mingw_64 \
   QTBINARYDIR=$HOME/Development/Qt6.5.3-linux/6.5.3/gcc_64/bin \
   ../kid3/build.sh)
(cd mingw64_qt5_build && \
   PATH=/opt/mxe/usr/bin:$PATH \
   COMPILER=cross-mingw \
   QTPREFIX=$HOME/Development/Qt5.15.2-mingw64/5.15.2/mingw81_64 \
   QTBINARYDIR=$HOME/Development/Qt5.15.2-linux/5.15.2/gcc_64/bin \
   ../kid3/build.sh && \
   origzip=(kid3/*-x64.zip) && \
   qt5zip=${origzip/-x64./-x64-Qt5.} && \
   mv $origzip $qt5zip)
(cd macos_build && \
   rm -f kid3/*-Darwin.dmg && \
   PATH=/opt/cmake/bin:$PATH \
   COMPILER=cross-macos \
   QTPREFIX=$HOME/Development/Qt6.5.3-mac/6.5.3/macos \
   OSXPREFIX=/opt/osxcross/target \
   QTBINARYDIR=$HOME/Development/Qt6.5.3-linux/6.5.3/gcc_64/bin \
   ../kid3/build.sh && \
   origdmg=(kid3/*-Darwin.dmg) && \
   qt6dmg=${origdmg/-Darwin./-Darwin-amd64.} && \
   mv $origdmg $qt6dmg)
(cd macos_qt5_build && \
   rm -f kid3/*-Darwin.dmg && \
   COMPILER=cross-macos \
   QTPREFIX=$HOME/Development/Qt5.15.2-mac/5.15.2/clang_64 \
   OSXPREFIX=/opt/osxcross/target \
   ../kid3/build.sh && \
   origdmg=(kid3/*-Darwin.dmg) && \
   qt5dmg=${origdmg/-Darwin./-Darwin-Qt5.} && \
   mv $origdmg $qt5dmg)
(cd android_build && \
   PATH=/opt/cmake/bin:$PATH \
   COMPILER=cross-android \
   QTPREFIX=$HOME/Development/Qt6.5.3-android/6.5.3/android_armv7 \
   QTBINARYDIR=$HOME/Development/Qt6.5.3-linux/6.5.3/gcc_64/bin \
   JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64 \
   ANDROID_SDK_ROOT=$HOME/Development/android-sdk \
   ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/23.1.7779620 \
   ../kid3/build.sh)
(cd android_arm64_build && \
   PATH=/opt/cmake/bin:$PATH \
   COMPILER=cross-android \
   QTPREFIX=$HOME/Development/Qt6.5.3-android64/6.5.3/android_arm64_v8a \
   QTBINARYDIR=$HOME/Development/Qt6.5.3-linux/6.5.3/gcc_64/bin \
   JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64 \
   ANDROID_SDK_ROOT=$HOME/Development/android-sdk \
   ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/23.1.7779620 \
   ../kid3/build.sh)
EOF
    chmod +x build-all.sh
  fi
  mkdir -p docker_image_context
  cd docker_image_context
  echo "### Build docker image"
  UNAME=$(uname)
  if hash podman 2>/dev/null; then
    DOCKER=podman
  elif [ -z "${UNAME##MINGW*}" ] || [ -z "${UNAME##MSYS*}" ] || id -Gn | grep -qw "docker\(root\)\?"; then
    DOCKER=docker
  else
    DOCKER="sudo docker"
  fi
  $DOCKER build --build-arg USER=${USER} --build-arg UID=$(id -u) -t ufleisch/kid3dev:bullseye . -f-<<"EOF"
FROM debian:bullseye-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
devscripts build-essential lintian debhelper extra-cmake-modules \
libkf5kio-dev libkf5doctools-dev qtmultimedia5-dev qtdeclarative5-dev \
qttools5-dev qttools5-dev-tools qtdeclarative5-dev-tools \
qml-module-qtquick2 cmake python libid3-3.8.3-dev libflac++-dev \
libvorbis-dev libtag1-dev libchromaprint-dev libavformat-dev \
libavcodec-dev docbook-xsl pkg-config libreadline-dev xsltproc \
debian-keyring dput-ng python3-distro-info sudo curl less \
locales ninja-build ccache p7zip-full genisoimage \
clang llvm nasm lib32z1 chrpath libpulse-mainloop-glib0 dmg2img archivemount \
openjdk-17-jre-headless libxcb-cursor0 libxrandr2
ARG USER
ARG UID
RUN adduser --quiet --disabled-password --uid $UID --gecos "User" $USER && \
echo "$USER:$USER" | chpasswd && usermod -aG sudo $USER && \
locale-gen en_US.UTF-8 && \
mkdir -p /home/$USER/projects/kid3 /home/$USER/Development && \
ln -s /proc/self/mounts /etc/mtab
USER $USER
CMD bash
EOF
  exit 0
fi

# Run docker image created with "makedocker".
if test "$1" = "rundocker"; then
  echo "### Run docker image"
  shift
  UNAME=$(uname)
  if hash podman 2>/dev/null; then
    DOCKER=podman
    USERNSARG=--userns=keep-id
  elif [ -z "${UNAME##MINGW*}" ] || [ -z "${UNAME##MSYS*}" ] || id -Gn | grep -qw "docker\(root\)\?"; then
    DOCKER=docker
    USERNSARG=
  else
    DOCKER="sudo docker"
    USERNSARG=
  fi
  $DOCKER run $USERNSARG --rm -it -e LANG=C.UTF-8 \
         --device /dev/fuse --cap-add SYS_ADMIN \
         -v $HOME/projects/kid3:$HOME/projects/kid3 \
         -v $HOME/.gradle:$HOME/.gradle \
         -v $HOME/.gnupg:$HOME/.gnupg:ro \
         -v /opt/cmake:/opt/cmake:ro \
         -v /opt/osxcross:/opt/osxcross:ro \
         -v /opt/mxe11:/opt/mxe11:ro \
         -v /opt/mxe:/opt/mxe:ro \
         -v $HOME/Development:$HOME/Development:ro ufleisch/kid3dev:bullseye "$@"
  exit 0
fi

# Build flatpak
if test "$1" = "flatpak"; then
  verify_not_in_srcdir
  echo "### Build flatpak"
  flatpak-builder --force-clean --ccache --repo=repo --subject="Build of org.kde.kid3 $(date --iso-8601=seconds)" app "$srcdir/packaging/flatpak/org.kde.kid3-local.json"
  exit 0
fi

# Build Debian package.
if test "$1" = "deb"; then
  verify_in_srcdir
  echo "### Build Debian package"
  shift
  test -d debian && rm -rf debian
  cp -R deb debian
  # The PPA version (e.g. trusty1) can be given as a parameter to prepare
  # a PPA upload. The source archive kid3_${version}.orig.tar.gz must be
  # in the parent directory.
  ppaversion=$1
  if test -n "$ppaversion"; then
    distribution=${ppaversion%%[0-9]*}
  else
    distribution=$(lsb_release -sc)
  fi

  if test -n "$ppaversion"; then
    version=$(sed -e 's/^kid3 (\([0-9\.-]\+\).*$/\1/;q' debian/changelog)
    DEBEMAIL="Urs Fleisch <ufleisch@users.sourceforge.net>" \
    dch --newversion=${version}${ppaversion} --distribution=$distribution --urgency=low \
    "No-change backport to $distribution."
    sed -i -e 's/^Maintainer:.*$/Maintainer: Urs Fleisch <ufleisch@users.sourceforge.net>/;/^Uploaders:/,+1d' debian/control
    debuild -S -sa &&
    echo "PPA upload ready for $distribution. Use:" &&
    echo "cd ..; dput ppa:ufleisch/kid3 kid3_${version}${ppaversion}_source.changes"
  else
    rm -rf debian/source debian/watch
    debuild
  fi
  exit 0
fi

if test "$1" = "clean"; then
  rm -rf zlib-${zlib_version}.dfsg libogg-${libogg_version} \
         libvorbis-${libvorbis_version} flac-${libflac_version%+ds*} \
         id3lib-${id3lib_version} taglib-${taglib_version} \
         ffmpeg-${ffmpeg_version} chromaprint-${chromaprint_version} \
         mp4v2-${mp4v2_version} utfcpp-${utfcpp_version} openssl-*
  exit 0
fi

if test "$1" = "getqt"; then
  if which wget >/dev/null; then
    DOWNLOAD=wget
  else
    DOWNLOAD="curl -skfLO"
  fi
  compiler=${COMPILER:-gcc}
  download_and_extract_qt
  exit 0
fi

# End of subtasks

verify_not_in_srcdir

target=${*:-libs package}

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
  qt_version=$(printf "%d.%d.%d" ${BASH_REMATCH[1]} ${BASH_REMATCH[2]} ${BASH_REMATCH[3]})
  qt_version_major=${BASH_REMATCH[1]}
else
  echo "Could not extract Qt version from $QTPREFIX, assuming $qt_version"
  if [[ "$qt_version" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
    qt_nr=$(printf "%d%02d%02d" ${BASH_REMATCH[1]} ${BASH_REMATCH[2]} ${BASH_REMATCH[3]})
    qt_version_major=${BASH_REMATCH[1]}
  fi
fi

if test "$compiler" = "cross-android" -o "$compiler" = "gcc-self-contained" && test "$qt_nr" -ge 60000; then
  openssl_version=3.0.9
elif test "$qt_nr" -ge 51204; then
  # Since Qt 5.12.4, OpenSSL 1.1.1 is supported
  openssl_version=1.1.1g
else
  openssl_version=1.0.2u
fi

if test "$compiler" = "gcc-self-contained"; then
  if test "$qt_nr" -lt 51500; then
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
    if hash ${cross_host}-gcc-posix 2>/dev/null; then
      _crossprefix=${cross_host}-
      # Qt uses posix threads, but x86_64-w64-mingw32-gcc uses win32 thread model.
      _crosssuffix=-posix
    elif hash ${cross_host}.shared-gcc 2>/dev/null; then
      _crossprefix=${cross_host}.shared-
      _crosssuffix=
    fi
  else
    cross_host="i686-w64-mingw32"
    _crossprefix=${cross_host}-
    _crosssuffix=
    # FFmpeg > 3 is not compatible with Windows XP
    ffmpeg_version=$ffmpeg3_version
    ffmpeg_patchlevel=$ffmpeg3_patchlevel
  fi
elif test "$compiler" = "cross-macos"; then
  osxprefix=${OSXPREFIX:-/opt/osxcross/target}
  # e.g. x86_64-apple-darwin17
  cross_host=$($osxprefix/bin/o64-clang --version | grep Target | cut -d' ' -f2)
  _crossprefix=${cross_host}-
  # e.g. $osxprefix/SDK/MacOSX10.13.sdk
  osxsdk=($osxprefix/SDK/*.sdk)
fi
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
  if test -z "${_android_qt_root%%*x86}"; then
    _android_abi=x86
    _android_prefix=i686-linux-android
  elif test -z "${_android_qt_root%%*x86_64}"; then
    _android_abi=x86_64
    _android_prefix=x86_64-linux-android
  elif test -z "${_android_qt_root%%*arm64_v8a}"; then
    _android_abi=arm64-v8a
    _android_prefix=aarch64-linux-android
  else
    _android_abi=armeabi-v7a
    _android_prefix=arm-linux-androideabi
  fi
fi

_chocoInstall=${ChocolateyInstall//\\/\/}
_chocoInstall=${_chocoInstall/C:/\/c}
for d in "$DOCBOOK_XSL_DIR" /usr/share/xml/docbook/stylesheet/nwalsh /usr/share/xml/docbook/xsl-stylesheets-* /usr/local/Cellar/docbook-xsl/*/docbook-xsl /opt/homebrew/Cellar/docbook-xsl/*/docbook-xsl $HOMEBREW_CELLAR/docbook-xsl/*/docbook-xsl /opt/local/share/xsl/docbook-xsl $_chocoInstall/lib/docbook-bundle/docbook-xsl-*; do
  if test -e $d/xhtml/docbook.xsl; then
    _docbook_xsl_dir=$d
    break
  fi
done

if test "$compiler" = "gcc-debug"; then
  export CFLAGS="-fPIC"
  export CXXFLAGS="-fPIC"
  FLAC_BUILD_OPTION="--enable-debug=yes"
  ID3LIB_BUILD_OPTION="--enable-debug=minimum"
  AV_BUILD_OPTION="--enable-debug=3 --enable-pic --extra-ldexeflags=-pie"
  CMAKE_BUILD_OPTION="-DCMAKE_BUILD_TYPE=Debug"
elif test "$compiler" = "gcc-self-contained"; then
  export CC=$gcc_self_contained_cc
  export CXX=$gcc_self_contained_cxx
  export CFLAGS="-O2 -fPIC"
  export CXXFLAGS="-O2 -fPIC"
  FLAC_BUILD_OPTION="--enable-debug=info CFLAGS=-O2 CXXFLAGS=-O2"
  ID3LIB_BUILD_OPTION="--enable-debug=minimum"
  AV_BUILD_OPTION="--enable-debug=1 --enable-pic --extra-ldexeflags=-pie"
  CMAKE_BUILD_OPTION="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
else
  FLAC_BUILD_OPTION="--enable-debug=info CFLAGS=-O2 CXXFLAGS=-O2"
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
  CMAKE_OPTIONS="-GNinja -DCMAKE_INSTALL_PREFIX=/usr/local"
  CONFIGURE_OPTIONS+=" --prefix=/usr/local"
elif test $kernel = "Darwin"; then
  CMAKE_OPTIONS="-GNinja"
  _macosx_version_min=10.7
else
  CMAKE_OPTIONS="-GNinja"
fi

if test "$compiler" = "cross-mingw"; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_TOOLCHAIN_FILE=$thisdir/mingw.cmake"
  CONFIGURE_OPTIONS="--host=${cross_host}"
  if test "$cross_host" = "x86_64-w64-mingw32"; then
    export CC=${_crossprefix}gcc${_crosssuffix}
    export CXX=${_crossprefix}g++${_crosssuffix}
  fi
elif test "$compiler" = "cross-macos"; then
  if test "$qt_nr" -ge 60500; then
    _macosx_version_min=10.15
  else
    _macosx_version_min=10.7
  fi
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_TOOLCHAIN_FILE=$thisdir/osxcross.cmake -DCMAKE_C_FLAGS=\"-O2 -mmacosx-version-min=${_macosx_version_min}\" -DCMAKE_CXX_FLAGS=\"-O2 -mmacosx-version-min=${_macosx_version_min} -fvisibility=hidden -fvisibility-inlines-hidden -stdlib=libc++\" -DCMAKE_EXE_LINKER_FLAGS=-stdlib=libc++ -DCMAKE_MODULE_LINKER_FLAGS=-stdlib=libc++ -DCMAKE_SHARED_LINKER_FLAGS=-stdlib=libc++"
  CONFIGURE_OPTIONS="--host=${cross_host}"
  export CC=${_crossprefix}clang
  export CXX=${_crossprefix}clang++
  export AR=${_crossprefix}ar
  export CFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=${_macosx_version_min}"
  export CXXFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=${_macosx_version_min} -stdlib=libc++"
  export LDFLAGS="$ARCH_FLAG -mmacosx-version-min=${_macosx_version_min} -stdlib=libc++"
fi

if test $kernel = "MINGW" || test "$compiler" = "cross-mingw"; then
  # Build zlib only for Windows.
  ZLIB_ROOT_PATH="$thisdir/buildroot/usr/local"
  TAGLIB_ZLIB_ROOT_OPTION="-DZLIB_ROOT=${ZLIB_ROOT_PATH}"
  CHROMAPRINT_ZLIB_OPTION="-DEXTRA_LIBS=\"-L${ZLIB_ROOT_PATH}/lib -lz\""
fi

if test $kernel = "Darwin"; then
  MACHINE_ARCH=$(uname -m)
  #MACHINE_ARCH=i386
  if test "$MACHINE_ARCH" = "i386"; then
    # To build a 32-bit Mac OS X version of Kid3 use:
    # cmake -GNinja -DCMAKE_CXX_FLAGS="-arch i386" -DCMAKE_C_FLAGS="-arch i386" -DCMAKE_EXE_LINKER_FLAGS="-arch i386" -DQT_QMAKE_EXECUTABLE=/usr/local/Trolltech/Qt-${qt_version}-i386/bin/qmake -DCMAKE_BUILD_TYPE=Release -DWITH_FFMPEG=ON -DCMAKE_INSTALL_PREFIX= ../kid3
    # Building multiple architectures needs ARCH_FLAG="-arch i386 -arch x86_64",
    # CONFIGURE_OPTIONS="--disable-dependency-tracking", but it fails with libav.
    ARCH=$MACHINE_ARCH
    ARCH_FLAG="-arch i386"
    export CC=gcc
    export CXX=g++
  elif test "$MACHINE_ARCH" = "arm64"; then
    # On Apple silicon, we can build for Intel or ARM. The environment variable
    # ARCH=x86_64 or ARCH=arm64 can be passed to this script.
    if test "$ARCH" != "x86_64"; then
      ARCH=$MACHINE_ARCH
    fi
    _macosx_version_min=10.15
    ARCH_FLAG="-arch $ARCH"
  else
    ARCH=$MACHINE_ARCH
    ARCH_FLAG="-Xarch_x86_64"
  fi
  if [[ $(sw_vers -productVersion) = 10.1* || $(sw_vers -productVersion) =~ ^1[1-9]\..*$ ]]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_C_FLAGS=\"-O2 $ARCH_FLAG -mmacosx-version-min=${_macosx_version_min}\" -DCMAKE_CXX_FLAGS=\"-O2 $ARCH_FLAG -mmacosx-version-min=${_macosx_version_min} -fvisibility=hidden -fvisibility-inlines-hidden -stdlib=libc++\" -DCMAKE_EXE_LINKER_FLAGS=\"$ARCH_FLAG -stdlib=libc++\" -DCMAKE_MODULE_LINKER_FLAGS=\"$ARCH_FLAG -stdlib=libc++\" -DCMAKE_SHARED_LINKER_FLAGS=\"$ARCH_FLAG -stdlib=libc++\""
    export CFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=${_macosx_version_min}"
    export CXXFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=${_macosx_version_min} -stdlib=libc++"
    export LDFLAGS="$ARCH_FLAG -mmacosx-version-min=${_macosx_version_min} -stdlib=libc++"
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
    elif test -d MSys2; then
      rm -rf usr
      mv MSys2/usr .
      rmdir MSys2
    fi
    cd ..
  fi
}

create_binary_archive() {
  if test -d "$2"; then
    find "$2" -type f -exec touch {} +
    tar czf "$1" "$2"
  fi
}

extract_binary_archive() {
  if test -f "$1"; then
    tar xzf "$1" -C $BUILDROOT
  fi
}

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
  if ! test -f $thisdir/mingw.cmake; then
    cat >$thisdir/mingw.cmake <<EOF
set(QT_PREFIX ${_qt_prefix})

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER ${_crossprefix}gcc${_crosssuffix})
set(CMAKE_CXX_COMPILER ${_crossprefix}g++${_crosssuffix})
set(CMAKE_RC_COMPILER ${_crossprefix}windres)
set(CMAKE_FIND_ROOT_PATH /usr/${cross_host} \${QT_PREFIX} $thisdir/buildroot/usr/local ${ZLIB_ROOT_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

EOF
    if test "$qt_version_major" = "6"; then
      _qt_libexec_dir=${_qt_bin_dir%bin}libexec
      cat >>$thisdir/mingw.cmake <<EOF
set(QT_BINARY_DIR ${_qt_bin_dir})
set(QT_LIBEXEC_DIR ${_qt_libexec_dir})
set(QT_LIBRARY_DIR  \${QT_PREFIX}/lib)
set(QT_QTCORE_LIBRARY   \${QT_PREFIX}/lib/libQt${qt_version_major}Core.a)
set(QT_QTCORE_INCLUDE_DIR \${QT_PREFIX}/include/QtCore)
set(QT_MKSPECS_DIR  \${QT_PREFIX}/mkspecs)
set(QT_MOC_EXECUTABLE  \${QT_LIBEXEC_DIR}/moc)
set(QT_UIC_EXECUTABLE  \${QT_LIBEXEC_DIR}/uic)

foreach (_exe moc rcc uic tracegen cmake_automoc_parser qlalr lprodump lrelease-pro lupdate-pro tracepointgen)
  if (NOT TARGET Qt${qt_version_major}::\${_exe})
    add_executable(Qt${qt_version_major}::\${_exe} IMPORTED)
    set_target_properties(Qt${qt_version_major}::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_LIBEXEC_DIR}/\${_exe}
    )
  endif ()
endforeach (_exe)
foreach (_exe lupdate lrelease windeployqt qtpaths androiddeployqt androidtestrunner lconvert)
  if (NOT TARGET Qt${qt_version_major}::\${_exe})
    add_executable(Qt${qt_version_major}::\${_exe} IMPORTED)
    set_target_properties(Qt${qt_version_major}::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_BINARY_DIR}/\${_exe}
    )
  endif ()
endforeach (_exe)
foreach (_exe qmake)
  if (NOT TARGET Qt${qt_version_major}::\${_exe})
    add_executable(Qt${qt_version_major}::\${_exe} IMPORTED)
    set_target_properties(Qt${qt_version_major}::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_PREFIX}/bin/\${_exe}
    )
  endif ()
endforeach (_exe)
EOF
    else
      cat >>$thisdir/mingw.cmake <<EOF
set(QT_BINARY_DIR ${_qt_bin_dir})
set(QT_LIBRARY_DIR  \${QT_PREFIX}/lib)
set(QT_QTCORE_LIBRARY   \${QT_PREFIX}/lib/libQt${qt_version_major}Core.a)
set(QT_QTCORE_INCLUDE_DIR \${QT_PREFIX}/include/QtCore)
set(QT_MKSPECS_DIR  \${QT_PREFIX}/mkspecs)
set(QT_MOC_EXECUTABLE  \${QT_BINARY_DIR}/moc)
set(QT_UIC_EXECUTABLE  \${QT_BINARY_DIR}/uic)

foreach (_exe moc rcc lupdate lrelease uic)
  if (NOT TARGET Qt${qt_version_major}::\${_exe})
    add_executable(Qt${qt_version_major}::\${_exe} IMPORTED)
    set_target_properties(Qt${qt_version_major}::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_BINARY_DIR}/\${_exe}
    )
  endif ()
endforeach (_exe)
EOF
    fi
  fi
elif test "$compiler" = "cross-macos"; then
  test -z ${PATH##$osxprefix/*} || PATH=$osxprefix/bin:$osxsdk/usr/bin:$PATH
  if ! test -f $thisdir/osxcross.cmake; then
    cat >$thisdir/osxcross.cmake <<EOF
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif (POLICY CMP0025)

set(QT_PREFIX $_qt_prefix)

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER $osxprefix/lib/ccache/bin/${_crossprefix}clang)
set(CMAKE_CXX_COMPILER $osxprefix/lib/ccache/bin/${_crossprefix}clang++)
set(CMAKE_FIND_ROOT_PATH $osxprefix/${cross_host};$osxsdk/usr;$osxprefix/${cross_host};$osxsdk;$osxsdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers;\${QT_PREFIX};$thisdir/buildroot/usr/local)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_AR:FILEPATH ${_crossprefix}ar)
set(CMAKE_RANLIB:FILEPATH ${_crossprefix}ranlib)
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")

EOF
    if test "$qt_version_major" = "6"; then
      _qt_libexec_dir=${_qt_bin_dir%bin}libexec
      cat >>$thisdir/osxcross.cmake <<EOF
set(QT_INCLUDE_DIRS_NO_SYSTEM ON)
set(QT_BINARY_DIR ${_qt_bin_dir})
set(QT_LIBEXEC_DIR ${_qt_libexec_dir})
set(QT_LIBRARY_DIR  \${QT_PREFIX}/lib)
set(Qt${qt_version_major}Core_DIR \${QT_PREFIX}/lib/cmake/Qt${qt_version_major}Core)
set(QT_MKSPECS_DIR  \${QT_PREFIX}/mkspecs)
set(QT_MOC_EXECUTABLE  \${QT_LIBEXEC_DIR}/moc)
set(QT_UIC_EXECUTABLE  \${QT_LIBEXEC_DIR}/uic)

foreach (_exe moc rcc uic tracegen cmake_automoc_parser qlalr lprodump lrelease-pro lupdate-pro tracepointgen)
  if (NOT TARGET Qt${qt_version_major}::\${_exe})
    add_executable(Qt${qt_version_major}::\${_exe} IMPORTED)
    set_target_properties(Qt${qt_version_major}::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_LIBEXEC_DIR}/\${_exe}
    )
  endif ()
endforeach (_exe)
foreach (_exe lupdate lrelease macdeployqt qtpaths androiddeployqt androidtestrunner lconvert)
  if (NOT TARGET Qt${qt_version_major}::\${_exe})
    add_executable(Qt${qt_version_major}::\${_exe} IMPORTED)
    set_target_properties(Qt${qt_version_major}::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_BINARY_DIR}/\${_exe}
    )
  endif ()
endforeach (_exe)
foreach (_exe qmake)
  if (NOT TARGET Qt${qt_version_major}::\${_exe})
    add_executable(Qt${qt_version_major}::\${_exe} IMPORTED)
    set_target_properties(Qt${qt_version_major}::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_PREFIX}/bin/\${_exe}
    )
  endif ()
endforeach (_exe)
EOF
    else
      cat >>$thisdir/osxcross.cmake <<EOF
set(QT_INCLUDE_DIRS_NO_SYSTEM ON)
set(QT_BINARY_DIR ${_qt_bin_dir})
set(QT_LIBRARY_DIR  \${QT_PREFIX}/lib)
set(Qt${qt_version_major}Core_DIR \${QT_PREFIX}/lib/cmake/Qt${qt_version_major}Core)
set(QT_MKSPECS_DIR  \${QT_PREFIX}/mkspecs)
set(QT_MOC_EXECUTABLE  \${QT_BINARY_DIR}/moc)
set(QT_UIC_EXECUTABLE  \${QT_BINARY_DIR}/uic)

foreach (_exe moc rcc lupdate lrelease uic)
  if (NOT TARGET Qt${qt_version_major}::\${_exe})
    add_executable(Qt${qt_version_major}::\${_exe} IMPORTED)
    set_target_properties(Qt${qt_version_major}::\${_exe} PROPERTIES
      IMPORTED_LOCATION \${QT_BINARY_DIR}/\${_exe}
    )
  endif ()
endforeach (_exe)
EOF
    fi
  fi
fi # cross-mingw, cross-macos

if [[ $target = *"libs"* ]]; then

# Download sources

test -d ${SRC_ARCHIVE_DIR:=$thisdir/source} || mkdir -p $SRC_ARCHIVE_DIR

download_taglib() {
  cd $SRC_ARCHIVE_DIR
  if test -n "${taglib_githash}"; then
    # Download an archive for a git hash
    if ! test -f taglib-${taglib_githash}.tar.gz; then
      $DOWNLOAD https://github.com/taglib/taglib/archive/${taglib_githash}.tar.gz
      mv ${taglib_githash}.tar.gz taglib-${taglib_githash}.tar.gz
    fi
  elif test -n "${taglib_version##v*}"; then
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
  cd -
}

download_utfcpp() {
  cd $SRC_ARCHIVE_DIR
  if ! test -f utfcpp-${utfcpp_version}.tar.gz; then
    $DOWNLOAD https://github.com/nemtrif/utfcpp/archive/refs/tags/v${utfcpp_version}.tar.gz
    mv v${utfcpp_version}.tar.gz utfcpp-${utfcpp_version}.tar.gz
  fi
  cd -
}

download_libflac() {
  cd $SRC_ARCHIVE_DIR
  test -f flac_${libflac_version}-${libflac_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/flac/flac_${libflac_version}-${libflac_patchlevel}.debian.tar.xz
  test -f flac_${libflac_version}.orig.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/flac/flac_${libflac_version}.orig.tar.xz
  cd -
}

download_id3lib() {
  cd $SRC_ARCHIVE_DIR
  test -f id3lib3.8.3_${id3lib_version}-${id3lib_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_${id3lib_version}-${id3lib_patchlevel}.debian.tar.xz
  test -f id3lib3.8.3_${id3lib_version}.orig.tar.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_${id3lib_version}.orig.tar.gz
  cd -
}

download_libogg() {
  cd $SRC_ARCHIVE_DIR
  test -f libogg_${libogg_version}-${libogg_patchlevel}.diff.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_${libogg_version}-${libogg_patchlevel}.diff.gz
  test -f libogg_${libogg_version}.orig.tar.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_${libogg_version}.orig.tar.gz
  cd -
}

download_libvorbis() {
  cd $SRC_ARCHIVE_DIR
  test -f libvorbis_${libvorbis_version}-${libvorbis_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_${libvorbis_version}-${libvorbis_patchlevel}.debian.tar.xz
  test -f libvorbis_${libvorbis_version}.orig.tar.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_${libvorbis_version}.orig.tar.gz
  cd -
}

download_zlib() {
  cd $SRC_ARCHIVE_DIR
  if test -n "$ZLIB_ROOT_PATH"; then
    test -f zlib_${zlib_version}.dfsg-${zlib_patchlevel}.debian.tar.xz ||
      $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_${zlib_version}.dfsg-${zlib_patchlevel}.debian.tar.xz
    test -f zlib_${zlib_version}.dfsg.orig.tar.bz2 ||
      $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_${zlib_version}.dfsg.orig.tar.bz2
  fi
  cd -
}

download_ffmpeg() {
  cd $SRC_ARCHIVE_DIR
  test -f ffmpeg_${ffmpeg_version}.orig.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/ffmpeg/ffmpeg_${ffmpeg_version}.orig.tar.xz
  test -f ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/ffmpeg/ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz
  cd -
}

download_chromaprint() {
  cd $SRC_ARCHIVE_DIR
  test -f chromaprint_${chromaprint_version}.orig.tar.gz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/c/chromaprint/chromaprint_${chromaprint_version}.orig.tar.gz
  test -f chromaprint_${chromaprint_version}-${chromaprint_patchlevel}.debian.tar.xz ||
    $DOWNLOAD http://ftp.de.debian.org/debian/pool/main/c/chromaprint/chromaprint_${chromaprint_version}-${chromaprint_patchlevel}.debian.tar.xz
  cd -
}

download_mp4v2() {
  cd $SRC_ARCHIVE_DIR
  test -f mp4v2-${mp4v2_version}.tar.bz2 ||
    $DOWNLOAD https://github.com/enzo1982/mp4v2/releases/download/v${mp4v2_version}/mp4v2-${mp4v2_version}.tar.bz2
  cd -
}

download_openssl() {
  cd $SRC_ARCHIVE_DIR
  # See http://doc.qt.io/qt-5/opensslsupport.html
  test -f Setenv-android.sh ||
    $DOWNLOAD https://wiki.openssl.org/images/7/70/Setenv-android.sh
  test -f openssl-${openssl_version}.tar.gz ||
    $DOWNLOAD https://www.openssl.org/source/openssl-${openssl_version}.tar.gz
  cd -
}

# Extract and patch sources

extract_utfcpp() {
  if ! test -d utfcpp-${utfcpp_version}; then
    echo "### Extracting utfcpp"
    tar xzf $SRC_ARCHIVE_DIR/utfcpp-${utfcpp_version}.tar.gz
  fi
}

extract_taglib() {
  if ! test -d taglib-${taglib_version}; then
    echo "### Extracting taglib"

    if test -n "${taglib_githash}"; then
      tar xzf $SRC_ARCHIVE_DIR/taglib-${taglib_githash}.tar.gz
      mv taglib-${taglib_githash} taglib-${taglib_version}
    else
      tar xzf $SRC_ARCHIVE_DIR/taglib-${taglib_version}.tar.gz
    fi
    cd taglib-${taglib_version}/
    if test "$cross_host" = "x86_64-w64-mingw32"; then
      if test -f $srcdir/packaging/patches/taglib-${taglib_githash}-win00-large_file.patch; then
        patch -p1 <$srcdir/packaging/patches/taglib-${taglib_githash}-win00-large_file.patch
      elif test -f $srcdir/packaging/patches/taglib-${taglib_version}-win00-large_file.patch; then
        patch -p1 <$srcdir/packaging/patches/taglib-${taglib_version}-win00-large_file.patch
      fi
    fi
    cd ..
  fi
}

extract_zlib() {
  if test -n "$ZLIB_ROOT_PATH"; then
    if ! test -d zlib-${zlib_version}; then
      echo "### Extracting zlib"

      tar xjf $SRC_ARCHIVE_DIR/zlib_${zlib_version}.dfsg.orig.tar.bz2
      cd zlib-${zlib_version}.dfsg/

      tar xJf $SRC_ARCHIVE_DIR/zlib_${zlib_version}.dfsg-${zlib_patchlevel}.debian.tar.xz || true
      echo Can be ignored: Cannot create symlink to debian.series
      if test -f debian/patches/debian.series; then
        for f in $(cat debian/patches/debian.series); do patch -p1 <debian/patches/$f; done
      fi
      cd ..
    fi
  fi
}

extract_libogg() {
  if ! test -d libogg-${libogg_version}; then
    echo "### Extracting libogg"

    tar xzf $SRC_ARCHIVE_DIR/libogg_${libogg_version}.orig.tar.gz
    cd libogg-${libogg_version}/
    gunzip -c $SRC_ARCHIVE_DIR/libogg_${libogg_version}-${libogg_patchlevel}.diff.gz | patch -p1
    if test $kernel = "Darwin" || test "$compiler" = "cross-macos"; then
      patch -p1 <$srcdir/packaging/patches/libogg-1.3.4-00-mac.patch
    fi
    cd ..
  fi
}

extract_libvorbis() {
  if ! test -d libvorbis-${libvorbis_version}; then
    echo "### Extracting libvorbis"

    tar xzf $SRC_ARCHIVE_DIR/libvorbis_${libvorbis_version}.orig.tar.gz
    cd libvorbis-${libvorbis_version}/
    tar xJf $SRC_ARCHIVE_DIR/libvorbis_${libvorbis_version}-${libvorbis_patchlevel}.debian.tar.xz
    if test -f debian/patches/series; then
      for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    fi
    test -f win32/VS2010/libogg.props.orig || mv win32/VS2010/libogg.props win32/VS2010/libogg.props.orig
    sed "s/<LIBOGG_VERSION>1.2.0</<LIBOGG_VERSION>$libogg_version</" win32/VS2010/libogg.props.orig >win32/VS2010/libogg.props
    cd ..
  fi
}

extract_libflac() {
  if ! test -d flac-${libflac_version%+ds*}; then
    echo "### Extracting libflac"

    tar xJf $SRC_ARCHIVE_DIR/flac_${libflac_version}.orig.tar.xz
    cd flac-${libflac_version%+ds*}/
    tar xJf $SRC_ARCHIVE_DIR/flac_${libflac_version}-${libflac_patchlevel}.debian.tar.xz
    if test -f debian/patches/series; then
      for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    fi
    patch -p1 <$srcdir/packaging/patches/flac-1.2.1-00-size_t_max.patch
    cd ..
  fi
}

extract_id3lib() {
  if ! test -d id3lib-${id3lib_version}; then
    echo "### Extracting id3lib"

    tar xzf $SRC_ARCHIVE_DIR/id3lib3.8.3_${id3lib_version}.orig.tar.gz
    cd id3lib-${id3lib_version}/
    tar xJf $SRC_ARCHIVE_DIR/id3lib3.8.3_${id3lib_version}-${id3lib_patchlevel}.debian.tar.xz
    if test -f debian/patches/series; then
      for f in $(cat debian/patches/series); do patch --binary -p1 <debian/patches/$f; done
    fi
    patch -p1 <$srcdir/packaging/patches/id3lib-3.8.3-win00-mingw.patch
    patch -p1 <$srcdir/packaging/patches/id3lib-3.8.3-win01-tempfile.patch
    test -f makefile.win32.orig || mv makefile.win32 makefile.win32.orig
    sed 's/-W3 -WX -GX/-W3 -EHsc/; s/-MD -D "WIN32" -D "_DEBUG"/-MDd -D "WIN32" -D "_DEBUG"/' makefile.win32.orig >makefile.win32
    cd ..
  fi
}

extract_ffmpeg() {
  if ! test -d ffmpeg-${ffmpeg_version}; then
    echo "### Extracting ffmpeg"

    tar xJf $SRC_ARCHIVE_DIR/ffmpeg_${ffmpeg_version}.orig.tar.xz || true
    cd ffmpeg-${ffmpeg_version}/
    tar xJf $SRC_ARCHIVE_DIR/ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz || true
    if test -f debian/patches/series; then
      for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    fi
    cd ..
  fi
}

extract_chromaprint() {
  if ! test -d chromaprint-${chromaprint_version}; then
    echo "### Extracting chromaprint"

    tar xzf $SRC_ARCHIVE_DIR/chromaprint_${chromaprint_version}.orig.tar.gz
    cd chromaprint-${chromaprint_version}/
    tar xJf $SRC_ARCHIVE_DIR/chromaprint_${chromaprint_version}-${chromaprint_patchlevel}.debian.tar.xz
    if test -f debian/patches/series; then
      for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
    fi
    cd ..
  fi
}

extract_mp4v2() {
  if ! test -d mp4v2-${mp4v2_version}; then
    echo "### Extracting mp4v2"

    tar xjf $SRC_ARCHIVE_DIR/mp4v2-${mp4v2_version}.tar.bz2
  fi
}

extract_openssl() {
  if ! test -d openssl-${openssl_version}; then
    echo "### Extracting openssl"

    tar xzf $SRC_ARCHIVE_DIR/openssl-${openssl_version}.tar.gz
    if test "$compiler" = "cross-android" && test "$qt_nr" -lt 60000; then
      cp $SRC_ARCHIVE_DIR/Setenv-android.sh openssl-${openssl_version}/
      cd openssl-${openssl_version}/
      sed -i 's/\r$//' Setenv-android.sh
      test -n "$ANDROID_NDK_ROOT" && \
        sed -i "s#^ANDROID_NDK_ROOT=.*#ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT#" \
          Setenv-android.sh
      chmod +x Setenv-android.sh
      patch -p0 <$srcdir/packaging/patches/openssl-1.1.1-android00-setenv.patch
      cd ..
    fi
  fi
}

# Build from sources

test -d ${BIN_ARCHIVE_DIR:=$thisdir/bin} || mkdir -p $BIN_ARCHIVE_DIR

if test "$compiler" = "cross-android"; then

  if test ! -f $BIN_ARCHIVE_DIR/openssl-${openssl_version}.tgz; then
    download_openssl
    extract_openssl
    echo "### Building OpenSSL"

    cd openssl-${openssl_version}/
    if test "$qt_nr" -lt 60000; then
      if test "$_android_abi" = "x86"; then
        sed -i 's/^_ANDROID_EABI=.*$/_ANDROID_EABI=x86-4.9/; s/^_ANDROID_ARCH=.*$/_ANDROID_ARCH=arch-x86/' Setenv-android.sh
      elif test "$_android_abi" = "x86_64"; then
        sed -i 's/^_ANDROID_EABI=.*$/_ANDROID_EABI=x86_64-4.9/; s/^_ANDROID_ARCH=.*$/_ANDROID_ARCH=arch-x86_64/' Setenv-android.sh
      elif test "$_android_abi" = "arm64-v8a"; then
        sed -i 's/^_ANDROID_EABI=.*$/_ANDROID_EABI=aarch64-linux-android-4.9/; s/^_ANDROID_ARCH=.*$/_ANDROID_ARCH=arch-arm64/' Setenv-android.sh
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
        elif test "$_android_abi" = "x86_64"; then
          sed -i 's/^CC=.*$/CC= x86_64-linux-android16-clang/; s/ -mandroid//' Makefile
        elif test "$_android_abi" = "arm64-v8a"; then
          sed -i 's/^CC=.*$/CC= aarch64-linux-android-clang/; s/ -mandroid//' Makefile
        else
          sed -i 's/^CC=.*$/CC= armv7a-linux-androideabi16-clang/; s/ -mandroid//' Makefile
        fi
      fi

      if test "${openssl_version:0:3}" = "1.0"; then
        make -j CALC_VERSIONS="SHLIB_COMPAT=; SHLIB_SOVER=" build_libs
        _ssl_lib_suffix=.so
      else
        make -j ANDROID_NDK_HOME=$ANDROID_NDK_ROOT SHLIB_VERSION_NUMBER= SHLIB_EXT=_1_1.so build_libs
        _ssl_lib_suffix=_1_1.so
      fi
    else
      PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH
      sed -i 's/shared_extension => ".so",/shared_extension => "_3.so",/' Configurations/15-android.conf
      if test "$_android_abi" = "x86"; then
        _os_compiler=android-x86
      elif test "$_android_abi" = "x86_64"; then
        _os_compiler=android-x86_64
      elif test "$_android_abi" = "arm64-v8a"; then
        _os_compiler=android-arm64
      else
        _os_compiler=android-arm
      fi
      ANDROID_NDK_HOME=$ANDROID_NDK_ROOT ./Configure shared $_os_compiler
      sed -i "s,^DESTDIR=\$,DESTDIR=$PWD/inst," Makefile
      make -j ANDROID_NDK_HOME=$ANDROID_NDK_ROOT SHLIB_VERSION_NUMBER= SHLIB_EXT=_3.so build_libs install_dev
      _ssl_lib_suffix=_3.so
      _android_prefix=llvm
    fi
    mkdir -p inst/usr/local/lib
    cp --dereference libssl${_ssl_lib_suffix} libcrypto${_ssl_lib_suffix} inst/usr/local/lib/
    $_android_prefix-strip -s inst/usr/local/lib/*.so
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/openssl-${openssl_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/openssl-${openssl_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/utfcpp-${utfcpp_version}.tgz; then
    download_utfcpp
    extract_utfcpp
    echo "### Building utfcpp"

    cd utfcpp-${utfcpp_version}/
    test -f build.ninja || eval cmake $CMAKE_BUILD_OPTION $CMAKE_OPTIONS -DUTF8_TESTS=OFF
    ninja
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/utfcpp-${utfcpp_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/utfcpp-${utfcpp_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/taglib-${taglib_version}.tgz; then
    download_taglib
    extract_taglib
    echo "### Building taglib"

    cd taglib-${taglib_version}/
    test -f build.ninja || cmake -GNinja -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_FIND_ROOT_PATH=$BUILDROOT/usr/local -DANDROID_NDK=$_android_ndk_root -DANDROID_ABI=$_android_abi -DCMAKE_TOOLCHAIN_FILE=$_android_toolchain_cmake -DANDROID_PLATFORM=$_android_platform -DANDROID_CCACHE=$_android_ccache
    ninja
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/taglib-${taglib_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/taglib-${taglib_version}.tgz

  # Set a QTPREFIX starting with "$(pwd)/qt-", e.g. $(pwd)/qt-6.5.3/6.5.3/android_arm64_v8a
  # or $(pwd)/qt-6.5.3/6.5.3/android_armv7 or $(pwd)/qt-6.5.3/6.5.3/android_x86_64
  # in order to download Qt binaries to the given $QTPREFIX if it does not already exist.
  if test "${QTPREFIX%%/qt-*}" = "$(pwd)" -a ! -d "$QTPREFIX"; then
    download_and_extract_qt
  fi

else #  cross-android

  if ( test "$compiler" = "gcc-self-contained" || test "$compiler" = "gcc-debug" ) \
       && test ! -f $BIN_ARCHIVE_DIR/openssl-${openssl_version}.tgz; then
    download_openssl
    extract_openssl
    echo "### Building OpenSSL"

    cd openssl-${openssl_version}
    ./Configure shared enable-ec_nistp_64_gcc_128 linux-x86_64 -Wa,--noexecstack
    make depend || true
    make -j build_libs
    mkdir -p inst/usr/local/ssl
    cp --dereference libssl.so libcrypto.so inst/usr/local/ssl/
    strip -s inst/usr/local/ssl/*.so
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/openssl-${openssl_version}.tgz usr
    cd ../..
  elif ( ( test "$compiler" = "cross-mingw" || test "$kernel" = "MINGW" ) && test "${openssl_version:0:3}" != "1.0" ) \
       && test ! -f $BIN_ARCHIVE_DIR/openssl-${openssl_version}.tgz; then
    download_openssl
    extract_openssl
    echo "### Building OpenSSL"

    cd openssl-${openssl_version}
    if test "$cross_host" = "x86_64-w64-mingw32" || [[ $(uname) =~ ^MINGW64 ]]; then
      _target=mingw64
    else
      _target=mingw
    fi
    if test -z "${cross_host}"; then
      _crossprefix=
    fi
    if test "$cross_host" = "x86_64-w64-mingw32"; then
      _cctmp=$CC
      CC=gcc${_crosssuffix}
    fi
    ./Configure shared enable-ec_nistp_64_gcc_128 $_target --cross-compile-prefix=$_crossprefix
    make depend || true
    make -j build_libs
    if test "$cross_host" = "x86_64-w64-mingw32"; then
      CC=$_cctmp
    fi
    mkdir -p inst/usr/local/ssl
    cp lib{ssl,crypto}*.dll inst/usr/local/ssl/
    ${_crossprefix}strip -s inst/usr/local/ssl/*.dll
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/openssl-${openssl_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/openssl-${openssl_version}.tgz

  if test -n "$ZLIB_ROOT_PATH" && test ! -f $BIN_ARCHIVE_DIR/zlib-${zlib_version}.tgz; then
    download_zlib
    extract_zlib
    echo "### Building zlib"

    cd zlib-${zlib_version}.dfsg/
    mkdir -p inst/usr/local
    if test $kernel = "MINGW"; then
      CFLAGS="$CFLAGS -g -O3 -Wall -DNO_FSEEKO" ./configure --static
      make -j install prefix=`pwd`/inst/usr/local
    elif test "$compiler" = "cross-mingw"; then
      CHOST=${_crossprefix} CFLAGS="$CFLAGS -g -O3 -Wall -DNO_FSEEKO" ./configure --static
      make -j install prefix=`pwd`/inst/usr/local
    else
      CFLAGS="$CFLAGS -g -O3 -Wall -DNO_FSEEKO" ./configure --static
      sed 's/LIBS=$(STATICLIB) $(SHAREDLIB) $(SHAREDLIBV)/LIBS=$(STATICLIB)/' Makefile >Makefile.inst
      make -j install -f Makefile.inst prefix=`pwd`/inst/usr/local
    fi
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/zlib-${zlib_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/zlib-${zlib_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/libogg-${libogg_version}.tgz; then
    download_libogg
    extract_libogg
    echo "### Building libogg"

    cd libogg-${libogg_version}/
    test -f build.ninja || eval cmake -DBUILD_SHARED_LIBS=OFF $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/libogg-${libogg_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/libogg-${libogg_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/libvorbis-${libvorbis_version}.tgz; then
    download_libvorbis
    extract_libvorbis
    echo "### Building libvorbis"

    cd libvorbis-${libvorbis_version}/
    test -f build.ninja || eval cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=$thisdir/buildroot/usr/local $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/libvorbis-${libvorbis_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/libvorbis-${libvorbis_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/flac-${libflac_version}.tgz; then
    download_libflac
    extract_libflac
    echo "### Building libflac"

    cd flac-${libflac_version%+ds*}/
    _cmake_args="-DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF -DINSTALL_MANPAGES=OFF -DCMAKE_PREFIX_PATH=$thisdir/buildroot/usr/local"
    if test $kernel = "MINGW" || test "$compiler" = "cross-mingw"; then
      # Avoid having to ship with libssp-0.dll
      _cmake_args+=" -DWITH_STACK_PROTECTOR=OFF -DWITH_FORTIFY_SOURCE=OFF"
    fi
    test -f build.ninja || eval cmake $_cmake_args $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/flac-${libflac_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/flac-${libflac_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/id3lib-${id3lib_version}.tgz && ! test $kernel = "Darwin" -a $(uname -m) = "arm64"; then
    download_id3lib
    extract_id3lib
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
    make -j install DESTDIR=`pwd`/inst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/id3lib-${id3lib_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/id3lib-${id3lib_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/utfcpp-${utfcpp_version}.tgz; then
    download_utfcpp
    extract_utfcpp
    echo "### Building utfcpp"

    cd utfcpp-${utfcpp_version}/
    test -f build.ninja || eval cmake $CMAKE_BUILD_OPTION $CMAKE_OPTIONS -DUTF8_TESTS=OFF
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/utfcpp-${utfcpp_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/utfcpp-${utfcpp_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/taglib-${taglib_version}.tgz; then
    download_taglib
    extract_taglib
    echo "### Building taglib"

    cd taglib-${taglib_version}/
    test -f build.ninja || eval cmake -DBUILD_SHARED_LIBS=OFF $TAGLIB_ZLIB_ROOT_OPTION $CMAKE_BUILD_OPTION $CMAKE_OPTIONS -DCMAKE_PREFIX_PATH=$BUILDROOT/usr/local
    ninja
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/taglib-${taglib_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/taglib-${taglib_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/ffmpeg-${ffmpeg_version}.tgz; then
    download_ffmpeg
    extract_ffmpeg
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
      AV_CONFIGURE_OPTIONS="--cross-prefix=${_crossprefix} --arch=x86 --target-os=mingw32 --sysinclude=/usr/${cross_host}/include"
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
      AV_CONFIGURE_OPTIONS="--disable-iconv --enable-cross-compile --cross-prefix=${_crossprefix} --arch=x86 --target-os=darwin --cc=$CC --cxx=$CXX"
    elif test "$compiler" = "gcc-debug" || test "$compiler" = "gcc-self-contained"; then
      test -n "$CC" && AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --cc=$CC"
      test -n "$CXX" && AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --cxx=$CXX"
    fi
    if test $kernel = "Darwin" || test $kernel = "MINGW"; then
      AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --disable-iconv"
    fi
    if test $kernel = "Darwin" && test $(uname -m) = "arm64"; then
      AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --arch=$ARCH"
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
    make -j V=1
    mkdir -p inst
    make install DESTDIR=`pwd`/inst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/ffmpeg-${ffmpeg_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/ffmpeg-${ffmpeg_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/chromaprint-${chromaprint_version}.tgz; then
    download_chromaprint
    extract_chromaprint
    echo "### Building chromaprint"

    # The zlib library path was added for MinGW-builds GCC 4.7.2.
    cd chromaprint-${chromaprint_version}/
    test -f build.ninja || eval cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTS=OFF $CHROMAPRINT_ZLIB_OPTION -DFFMPEG_ROOT=$thisdir/buildroot/usr/local $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/chromaprint-${chromaprint_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/chromaprint-${chromaprint_version}.tgz

  if test ! -f $BIN_ARCHIVE_DIR/mp4v2-${mp4v2_version}.tgz; then
    download_mp4v2
    extract_mp4v2
    echo "### Building mp4v2"

    cd mp4v2-${mp4v2_version}/
    test -f build.ninja || eval cmake -DBUILD_SHARED=OFF -DBUILD_UTILS=OFF -DBUILD_SHARED_LIBS=OFF $CMAKE_BUILD_OPTION $CMAKE_OPTIONS
    DESTDIR=`pwd`/inst ninja install
    fixcmakeinst
    cd inst
    create_binary_archive $BIN_ARCHIVE_DIR/mp4v2-${mp4v2_version}.tgz usr
    cd ../..
  fi
  extract_binary_archive $BIN_ARCHIVE_DIR/mp4v2-${mp4v2_version}.tgz

  # Set a QTPREFIX starting with "$(pwd)/qt-", e.g. $(pwd)/qt-6.5.3/6.5.3/gcc_64
  # for Linux or $(pwd)/qt-6.5.3/6.5.3/macos for macOS in order to download Qt
  # binaries to the given $QTPREFIX if it does not already exist.
  if test "${QTPREFIX%%/qt-*}" = "$(pwd)" -a ! -d "$QTPREFIX"; then
    download_and_extract_qt
  fi
fi # cross-android, else
fi # libs

if [[ $target = *"package"* ]]; then

  if test "$compiler" = "cross-android"; then

    if ! test -d kid3; then
      echo "### Creating kid3 build directory"
      mkdir kid3
      test -e $HOME/Development/ufleisch-release-key.keystore && cp -s $HOME/Development/ufleisch-release-key.keystore kid3/
      cat >kid3/run-cmake.sh <<EOF
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
cmake -GNinja -DJAVA_HOME=\$_java_root -DQT_ANDROID_SDK_ROOT=\$_android_sdk_root -DANDROID_SDK_ROOT=\$_android_sdk_root -DANDROID_NDK=\$_android_ndk_root -DAPK_ALL_TARGET=OFF -DANDROID_ABI=\$_android_abi -DANDROID_EXTRA_LIBS_DIR=\$_buildprefix/lib -DANDROID_KEYSTORE_PATH=\$_android_keystore_path -DANDROID_KEYSTORE_ALIAS=\$_android_keystore_alias -DCMAKE_TOOLCHAIN_FILE=$_android_toolchain_cmake -DANDROID_PLATFORM=$_android_platform -DANDROID_CCACHE=$_android_ccache -DQT_QMAKE_EXECUTABLE=\$_android_qt_root/bin/qmake -DQT_HOST_PATH=${QTBINARYDIR%/bin} -DCMAKE_BUILD_TYPE=Release -DCMAKE_FIND_ROOT_PATH=\$_buildprefix -DDOCBOOK_XSL_DIR=${_docbook_xsl_dir} -DPYTHON_EXECUTABLE=/usr/bin/python -DXSLTPROC=/usr/bin/xsltproc -DGZIP_EXECUTABLE=/bin/gzip $srcdir
EOF
      chmod +x kid3/run-cmake.sh
    fi

  else # cross-android

    if ! test -d kid3; then
      echo "### Creating kid3 build directory"

      mkdir kid3
      if test "$compiler" = "cross-mingw"; then
        cat >kid3/run-cmake.sh <<EOF
#!/bin/bash
cmake -GNinja $CMAKE_BUILD_OPTION -DCMAKE_TOOLCHAIN_FILE=$thisdir/mingw.cmake -DCMAKE_INSTALL_PREFIX= -DCMAKE_CXX_FLAGS="-g -O2 -DMP4V2_USE_STATIC_LIB -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL -DTAGLIB_STATIC" -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DDOCBOOK_XSL_DIR=${_docbook_xsl_dir} ../../kid3
EOF
      elif test "$compiler" = "cross-macos"; then
        cat >kid3/run-cmake.sh <<EOF
#!/bin/bash
test -z \${PATH##$osxprefix/*} || PATH=$osxprefix/bin:$osxsdk/usr/bin:\$PATH
cmake -GNinja $CMAKE_BUILD_OPTION -DREMOVE_ARCH=arm64 -DCMAKE_TOOLCHAIN_FILE=$thisdir/osxcross.cmake -DCMAKE_INSTALL_PREFIX= -DCMAKE_CXX_FLAGS="-g -O2 -DMP4V2_USE_STATIC_LIB" -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DDOCBOOK_XSL_DIR=${_docbook_xsl_dir} ../../kid3
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
        cat >kid3/run-cmake.sh <<EOF
#!/bin/bash
BUILDPREFIX=\$(cd ..; pwd)/buildroot/usr/local
export PKG_CONFIG_PATH=\$BUILDPREFIX/lib/pkgconfig
cmake -GNinja $CMAKE_BUILD_OPTION -DCMAKE_CXX_COMPILER=${gcc_self_contained_cxx} -DCMAKE_C_COMPILER=${gcc_self_contained_cc} -DQT_QMAKE_EXECUTABLE=${_qt_prefix}/bin/qmake -DWITH_READLINE=OFF -DLINUX_SELF_CONTAINED=ON -DWITH_QML=ON -DCMAKE_PREFIX_PATH=\$BUILDPREFIX -DWITH_FFMPEG=ON -DFFMPEG_ROOT=\$BUILDPREFIX -DWITH_MP4V2=ON -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. -DWITH_LIBDIR=. -DWITH_PLUGINSDIR=./plugins -DWITH_DOCBOOKDIR=${_docbook_xsl_dir} ../../kid3
EOF
      elif test $kernel = "Darwin" -a "$(uname -m)" = "arm64"; then
        if test "$ARCH" = "arm64"; then
          _other_arch=x86_64
        else
          _other_arch=arm64
        fi
        _qt_prefix=${QTPREFIX:-/usr/local/Trolltech/Qt${qt_version}/${qt_version}/clang_64}
        cat >kid3/run-cmake.sh <<EOF
#!/bin/bash
INCLUDE=../buildroot/usr/local/include LIB=../buildroot/usr/local/lib cmake $CMAKE_BUILD_OPTION $CMAKE_OPTIONS -DCMAKE_APPLE_SILICON_PROCESSOR=$ARCH -DREMOVE_ARCH=$_other_arch -DQT_QMAKE_EXECUTABLE=${_qt_prefix}/bin/qmake -DCMAKE_INSTALL_PREFIX= -DCMAKE_PREFIX_PATH=$thisdir/buildroot/usr/local -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DWITH_ID3LIB=OFF -DWITH_DOCBOOKDIR=${_docbook_xsl_dir} ../../kid3
EOF
      elif test $kernel = "Darwin"; then
        _qt_prefix=${QTPREFIX:-/usr/local/Trolltech/Qt${qt_version}/${qt_version}/clang_64}
        cat >kid3/run-cmake.sh <<EOF
#!/bin/bash
INCLUDE=../buildroot/usr/local/include LIB=../buildroot/usr/local/lib cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQT_QMAKE_EXECUTABLE=${_qt_prefix}/bin/qmake -DCMAKE_INSTALL_PREFIX= -DCMAKE_PREFIX_PATH=$thisdir/buildroot/usr/local -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DWITH_DOCBOOKDIR=${_docbook_xsl_dir} ../../kid3
EOF
      elif test $kernel = "MINGW"; then
        _qtToolsMingw=($QTPREFIX/../../Tools/mingw*)
        _qtToolsMingw=$(realpath $_qtToolsMingw)
        cat >kid3/run-cmake.sh <<EOF
#!/bin/bash
INCLUDE=../buildroot/usr/local/include LIB=../buildroot/usr/local/lib cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQT_QMAKE_EXECUTABLE=${QTPREFIX}/bin/qmake -DCMAKE_INSTALL_PREFIX= -DCMAKE_PREFIX_PATH=$thisdir/buildroot/usr/local -DCMAKE_CXX_FLAGS="-g -O2 -DMP4V2_USE_STATIC_LIB -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL -DTAGLIB_STATIC" -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DWITH_DOCBOOKDIR=${_docbook_xsl_dir} ../../kid3
EOF
        _qtPrefixWin=${QTPREFIX//\//\\}
        _qtPrefixWin=${_qtPrefixWin/\\c/C:}
        _qtToolsMingwWin=${_qtToolsMingw//\//\\}
        _qtToolsMingwWin=${_qtToolsMingwWin/\\c/C:}
        cat >kid3/build.bat <<EOF
set INCLUDE=../buildroot/usr/local/include
set LIB=../buildroot/usr/local/lib
echo ;%PATH%; | find /C /I ";$_qtPrefixWin\bin;"
if errorlevel 1 (
path $_qtPrefixWin\bin;$_qtToolsMingwWin\bin;$_qtToolsMingwWin\opt\bin;C:\msys64\mingw64\bin;%PATH%
)
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX= -DCMAKE_PREFIX_PATH=%cd%/../buildroot/usr/local -DCMAKE_CXX_FLAGS="-g -O2 -DMP4V2_USE_STATIC_LIB -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL -DTAGLIB_STATIC" -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DWITH_DOCBOOKDIR="C:\msys64\usr\share\xml\docbook\xsl-stylesheets-1.79.2" -DXSLTPROC=C:/msys64/usr/bin/xsltproc.exe ../../kid3
cmake --build .
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
        cat >kid3/run-cmake.sh <<EOF
#!/bin/bash
BUILDPREFIX=\$(cd ..; pwd)/buildroot/usr/local
export PKG_CONFIG_PATH=\$BUILDPREFIX/lib/pkgconfig
cmake -GNinja -DQT_QMAKE_EXECUTABLE=${QTPREFIX:-$QT_PREFIX}/bin/qmake -DLINUX_SELF_CONTAINED=ON -DWITH_READLINE=OFF -DWITH_QML=ON -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DMP4V2_USE_STATIC_LIB -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL -DTAGLIB_STATIC" -DCMAKE_PREFIX_PATH=\$BUILDPREFIX -DWITH_FFMPEG=ON -DFFMPEG_ROOT=\$BUILDPREFIX -DWITH_MP4V2=ON $CMAKE_BUILD_OPTION -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. -DWITH_LIBDIR=. -DWITH_PLUGINSDIR=./plugins -DWITH_DOCBOOKDIR=${_docbook_xsl_dir} ../../kid3
EOF
      else
        cat >kid3/run-cmake.sh <<EOF
#!/bin/bash
BUILDPREFIX=\$(cd ..; pwd)/buildroot/usr/local
export PKG_CONFIG_PATH=\$BUILDPREFIX/lib/pkgconfig
cmake -GNinja -DLINUX_SELF_CONTAINED=ON -DWITH_QML=ON -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DMP4V2_USE_STATIC_LIB -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL -DTAGLIB_STATIC" -DCMAKE_PREFIX_PATH=\$BUILDPREFIX -DWITH_FFMPEG=ON -DFFMPEG_ROOT=\$BUILDPREFIX -DWITH_MP4V2=ON $CMAKE_BUILD_OPTION -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. -DWITH_LIBDIR=. -DWITH_PLUGINSDIR=./plugins -DWITH_DOCBOOKDIR=${_docbook_xsl_dir} ../../kid3
EOF
      fi
      chmod +x kid3/run-cmake.sh
    fi # test -d kid3

  fi # cross-android, else

  echo "### Building kid3 package"

  pushd kid3 >/dev/null
  if test -f run-cmake.sh && ! test -f Makefile && ! test -f build.ninja; then
    ./run-cmake.sh
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
    for f in Qt${qt_version_major}Core.dll Qt${qt_version_major}Network.dll Qt${qt_version_major}Gui.dll Qt${qt_version_major}Xml.dll Qt${qt_version_major}Widgets.dll Qt${qt_version_major}Multimedia.dll Qt${qt_version_major}Qml.dll Qt${qt_version_major}Quick.dll $_gccDll libstdc++-6.dll libwinpthread-1.dll; do
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
    test -z ${PATH##$osxprefix/*} || PATH=$osxprefix/bin:$osxsdk/usr/bin:$PATH
    rm -rf inst
    DESTDIR=$(pwd)/inst ninja install/strip
    ln -s /Applications inst/Applications
    genisoimage -V "Kid3" -D -R -apple -no-pad -o uncompressed.dmg inst
    _version=$(grep VERSION config.h | cut -d'"' -f2)
    dmg dmg uncompressed.dmg kid3-$_version-Darwin.dmg
    rm uncompressed.dmg
  elif test "$compiler" = "cross-android"; then
    export JAVA_HOME=$(grep _java_root= run-cmake.sh | cut -d'=' -f2)
    export QT_ANDROID_KEYSTORE_PATH=$(grep ANDROID_KEYSTORE_PATH CMakeCache.txt | cut -d= -f2)
    export QT_ANDROID_KEYSTORE_ALIAS=$(grep ANDROID_KEYSTORE_ALIAS CMakeCache.txt | cut -d= -f2)
    # Interactive input is not possible when building with Ninja.
    # Prompt the user for the password at the beginning. To avoid this prompt,
    # define the QT_ANDROID_KEYSTORE_STORE_PASS environment variable.
    if test -n "$QT_ANDROID_KEYSTORE_PATH" && test -n "$QT_ANDROID_KEYSTORE_ALIAS" &&
       test -z "${QT_ANDROID_KEYSTORE_STORE_PASS+x}"; then
      read -p 'Android signing password: ' -s QT_ANDROID_KEYSTORE_STORE_PASS
      echo
      export QT_ANDROID_KEYSTORE_STORE_PASS
    fi
    ninja apk
    _version=$(grep VERSION config.h | cut -d'"' -f2)
    for prefix in android/build/outputs/apk/release/android-release android/build/outputs/apk/android-release android/bin/QtApp-release android/android-build/build/outputs/apk/release/android-build-release android/android-build/build/outputs/apk/debug/android-build-debug; do
      for suffix in signed unsigned; do
        _apkpath=${prefix}-${suffix}.apk
        if test -f $_apkpath; then
          cp -a $_apkpath kid3-$_version-android-$_android_abi.apk
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
      make -j package
    fi
  fi
  popd >/dev/null
fi # package

trap - ERR

echo "### Built successfully"
