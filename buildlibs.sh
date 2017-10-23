#!/bin/bash
# This script can be used to build static libraries for the Windows and Mac
# versions of Kid3. Linux and BSD users do not need it because the libraries
# can be installed from their repositories.
#
# First you have to install the necessary tools:
#
# For Windows:
#
# Building the libraries needs msys/MinGW, CMake, yasm.
# You should use the MinGW which comes with Qt and add msys tools to build
# the libraries.
# Download yasm from
# http://www.tortall.net/projects/yasm/releases/yasm-1.2.0-win32.exe
# and copy it into msys /bin as yasm.exe.
# Start the msys shell, add cmake to the path and start this script.
# When the script has run successfully, the libraries are installed below
# /usr/local/ in msys. You can then proceed to the Kid3 build.
#
# Building Kid3 needs MinGW, CMake, Qt, xsltproc, html\docbook.xsl, dumpbin.
# Dumpbin is needed for the final packages and can be found in the MS SDK or
# MS Visual C++ Express Edition. Set the environment variables in
# win32/buildkid3.bat, so that these tools can be found, then start
# buildkid3.bat from a Windows command prompt.
#
# You can also build a Windows version from Linux using the MinGW cross
# compiler. Set compiler="cross-mingw" below.
#
# For Mac: XCode, Qt, html\docbook.xsl. XCode and Qt should be installed at
# the default location, docbook.xsl in
# $HOME/docbook-xsl-1.72.0/html/docbook.xsl.
#
# To build for Android, set compiler="cross-android".
#
# The source code for the libraries is downloaded from Debian and Ubuntu
# repositories. If the files are no longer available, use a later version,
# it should still work.
#
# buildlibs.sh will download, build and install zlib, libogg, libvorbis,
# flac, id3lib, taglib, ffmpeg, chromaprint. You are then ready to build Kid3
# from the win32 or macosx directories by starting buildkid3.bat (Windows) or
# buildkid3.sh (Mac).

# Exit if an error occurs
set -e

thisdir=$(pwd)

kernel=$(uname)
test ${kernel:0:5} = "MINGW" && kernel="MINGW"

compiler="gcc"

qt_version=5.6.3
zlib_version=1.2.8
zlib_patchlevel=5
libogg_version=1.3.2
libogg_patchlevel=1
libvorbis_version=1.3.5
libvorbis_patchlevel=4
ffmpeg_version=3.4
ffmpeg_patchlevel=1
#libav_version=11.4
#libav_patchlevel=2
libflac_version=1.3.2
libflac_patchlevel=1
id3lib_version=3.8.3
id3lib_patchlevel=16.2
taglib_version=1.11.1
chromaprint_version=1.4.2
chromaprint_patchlevel=1
mp4v2_version=2.0.0
mp4v2_patchlevel=6

FLAC_BUILD_OPTION="--enable-debug"
ID3LIB_BUILD_OPTION="--enable-debug=minimum"
AV_BUILD_OPTION="--enable-debug=1"
CMAKE_BUILD_OPTION="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
# Uncomment for debug build
#FLAC_BUILD_OPTION="--enable-debug"
#ID3LIB_BUILD_OPTION="--enable-debug=yes"
#AV_BUILD_OPTION="--enable-debug=3"
#CMAKE_BUILD_OPTION="-DCMAKE_BUILD_TYPE=Debug"

# Uncomment for a LINUX_SELF_CONTAINED build
#export CC="gcc-4.8"
#export CXX="g++-4.8"
#export CFLAGS="-O2 -fPIC"
#export CXXFLAGS="-O2 -fPIC"
#AV_BUILD_OPTION="$AV_BUILD_OPTION --enable-pic --extra-ldexeflags=-pie"

if ! which cmake >/dev/null; then
  echo cmake not found.
  return
  exit 1
fi

if test $kernel = "MSYS_NT-6.1"; then
kernel="MINGW"
CONFIGURE_OPTIONS="--build=x86_64-w64-mingw32 --target=i686-w64-mingw32"
fi
if test $kernel = "MINGW"; then
CMAKE_OPTIONS="-G \"MSYS Makefiles\" -DCMAKE_INSTALL_PREFIX=/usr/local"
elif test $kernel = "Darwin"; then
CMAKE_OPTIONS="-G \"Unix Makefiles\""
fi

if test "$compiler" = "cross-mingw"; then
CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_TOOLCHAIN_FILE=$thisdir/source/mingw.cmake"

# Note that Ubuntu i686-w64-mingw32 is incompatible (sjlj) with the mingw (dw2)
# used for the Qt mingw binaries >=4.8.6.
# I am using a custom built cross mingw (--disable-sjlj-exceptions --enable-threads=posix)
cross_host="i686-w64-mingw32"
CONFIGURE_OPTIONS="--host=${cross_host}"
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
fi

if which wget >/dev/null; then
DOWNLOAD=wget
else
DOWNLOAD="curl -skfLO"
fi

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

test -f taglib-${taglib_version}.tar.gz ||
$DOWNLOAD http://taglib.github.io/releases/taglib-${taglib_version}.tar.gz

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

# Create patch files

if test "$compiler" = "cross-mingw" && ! test -f mingw.cmake; then
  for d in $thisdir/qtbase5-dev-tools* /usr/lib/${HOSTTYPE/i686/i386}-linux-gnu/qt5/bin /usr/bin; do
    if test -x $d/moc; then
      _qt_bin_dir=$d
      break
    fi
  done
  cat >mingw.cmake <<EOF
set(QT_PREFIX /windows/Qt/Qt${qt_version}/${qt_version}/mingw49_32)

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
fi

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

cd ..


# Extract and patch sources

if test -n "$ZLIB_ROOT_PATH"; then
echo "### Extracting zlib"

if ! test -d zlib-${zlib_version}; then
tar xzf source/zlib_${zlib_version}.dfsg.orig.tar.gz
cd zlib-${zlib_version}/

unxz -c ../source/zlib_${zlib_version}.dfsg-${zlib_patchlevel}.debian.tar.xz | tar x || true
echo Can be ignored: Cannot create symlink to debian.series
for f in $(cat debian/patches/debian.series); do patch -p1 <debian/patches/$f; done
cd ..
fi
fi

echo "### Extracting libogg"

if ! test -d libogg-${libogg_version}; then
tar xzf source/libogg_${libogg_version}.orig.tar.gz
cd libogg-${libogg_version}/
gunzip -c ../source/libogg_${libogg_version}-${libogg_patchlevel}.diff.gz | patch -p1
cd ..
fi

echo "### Extracting libvorbis"

if ! test -d libvorbis-${libvorbis_version}; then
tar xzf source/libvorbis_${libvorbis_version}.orig.tar.gz
cd libvorbis-${libvorbis_version}/
unxz -c ../source/libvorbis_${libvorbis_version}-${libvorbis_patchlevel}.debian.tar.xz | tar x
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
if test $libvorbis_version = "1.3.5"; then
  patch -p1 <../source/vorbis_alloc_on_heap.patch
fi
test -f win32/VS2010/libogg.props.orig || mv win32/VS2010/libogg.props win32/VS2010/libogg.props.orig
sed "s/<LIBOGG_VERSION>1.2.0</<LIBOGG_VERSION>$libogg_version</" win32/VS2010/libogg.props.orig >win32/VS2010/libogg.props
cd ..
fi

echo "### Extracting libflac"

if ! test -d flac-${libflac_version}; then
unxz -c source/flac_${libflac_version}.orig.tar.xz | tar x
cd flac-${libflac_version}/
unxz -c ../source/flac_${libflac_version}-${libflac_patchlevel}.debian.tar.xz | tar x
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
patch -p1 <../source/flac_1.2.1_size_t_max_patch.diff
if test $kernel = "Darwin"; then
patch -p1 <../source/fink_flac.patch
patch -p0 <patches/nasm.h.patch
fi
cd ..
fi

echo "### Extracting id3lib"

if ! test -d id3lib-${id3lib_version}; then
tar xzf source/id3lib3.8.3_${id3lib_version}.orig.tar.gz
cd id3lib-${id3lib_version}/
unxz -c ../source/id3lib3.8.3_${id3lib_version}-${id3lib_patchlevel}.debian.tar.xz | tar x
for f in $(cat debian/patches/series); do patch --binary -p1 <debian/patches/$f; done
patch -p1 <../source/id3lib-3.8.3_mingw.patch
patch -p1 <../source/id3lib-3.8.3_wintempfile.patch
test -f makefile.win32.orig || mv makefile.win32 makefile.win32.orig
sed 's/-W3 -WX -GX/-W3 -EHsc/; s/-MD -D "WIN32" -D "_DEBUG"/-MDd -D "WIN32" -D "_DEBUG"/' makefile.win32.orig >makefile.win32
cd ..
fi

echo "### Extracting taglib"

if ! test -d taglib-${taglib_version}; then
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
fi
cd ..
fi

echo "### Extracting ffmpeg"

if test -n "${ffmpeg_version}"; then
unxz -c source/ffmpeg_${ffmpeg_version}.orig.tar.xz | tar x || true
cd ffmpeg-${ffmpeg_version}/
unxz -c ../source/ffmpeg_${ffmpeg_version}-${ffmpeg_patchlevel}.debian.tar.xz | tar x
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
if test $ffmpeg_version = "3.1.3"; then
  patch -p1 <../source/ffmpeg_mingw.patch
fi
cd ..
else
if test "${libav_version%.*}" = "0.8"; then
if ! test -d libav-${libav_version}; then
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
unxz -c source/libav_${libav_version}.orig.tar.xz | tar x || true
echo Can be ignored: Cannot create symlink to README.md
cd libav-${libav_version}/
unxz -c ../source/libav_${libav_version}-${libav_patchlevel}.debian.tar.xz | tar x
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

echo "### Extracting chromaprint"

if ! test -d chromaprint-${chromaprint_version}; then
tar xzf source/chromaprint_${chromaprint_version}.orig.tar.gz
cd chromaprint-${chromaprint_version}/
unxz -c ../source/chromaprint_${chromaprint_version}-${chromaprint_patchlevel}.debian.tar.xz | tar x
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
cd ..
fi

echo "### Extracting mp4v2"

if ! test -d mp4v2-${mp4v2_version}; then
tar xjf source/mp4v2_${mp4v2_version}~dfsg0.orig.tar.bz2
cd mp4v2-${mp4v2_version}/
unxz -c ../source/mp4v2_${mp4v2_version}~dfsg0-${mp4v2_patchlevel}.debian.tar.xz | tar x
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
if test $kernel = "MINGW" || test "$compiler" = "cross-mingw"; then
patch -p1 <../source/mp4v2_win32.patch
fi
cd ..
fi


# Build from sources

test -d bin || mkdir bin

for d in /usr/share/xml/docbook/stylesheet/nwalsh /usr/share/xml/docbook/xsl-stylesheets-*; do
  if test -e $d/html/docbook.xsl; then
    _docbook_xsl_dir=$d
    break
  fi
done

if test "$compiler" = "cross-android"; then

echo "### Building taglib"

_android_sdk_root=/opt/android/sdk
_android_ndk_root=$_android_sdk_root/ndk-bundle
_android_abi=armeabi-v7a
_android_toolchain_prefix=
_android_qt_root=/opt/Qt/5.5/android_armv7
#_android_abi=x86
#_android_toolchain_prefix=x86
#_android_qt_root=/opt/Qt/5.5/android_x86
test -d buildroot || mkdir buildroot
_buildroot=$(pwd)/buildroot

cd taglib-${taglib_version}/
cmake -DWITH_ASF=ON -DWITH_MP4=ON $taglib_static_option -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$_buildroot/usr/local -DANDROID_NDK=$_android_ndk_root -DANDROID_ABI=$_android_abi -DANDROID_TOOLCHAIN_PREFIX=$_android_toolchain_prefix -DCMAKE_TOOLCHAIN_FILE=../../kid3/android/qt-android-cmake/toolchain/android.toolchain.cmake
make install
cd ..

if ! test -d kid3; then
  mkdir kid3
  cat >kid3/build.sh <<EOF
_java_root=/usr/lib/jvm/java-7-openjdk-amd64
_android_sdk_root=$_android_sdk_root
_android_ndk_root=$_android_ndk_root
_android_abi=$_android_abi
_android_toolchain_prefix=$_android_toolchain_prefix
_android_qt_root=$_android_qt_root
_buildprefix=\$(cd ..; pwd)/buildroot/usr/local
cmake -DJAVA_HOME=\$_java_root -DQT_ANDROID_SDK_ROOT=\$_android_sdk_root -DANDROID_NDK=\$_android_ndk_root -DQT_ANDROID_ANT=/usr/bin/ant -DAPK_ALL_TARGET=OFF -DANDROID_ABI=\$_android_abi -DANDROID_TOOLCHAIN_PREFIX=\$_android_toolchain_prefix -DCMAKE_TOOLCHAIN_FILE=../../kid3/android/qt-android-cmake/toolchain/android.toolchain.cmake -DQT_QMAKE_EXECUTABLE=\$_android_qt_root/bin/qmake -DCMAKE_BUILD_TYPE=Release -DDOCBOOK_XSL_DIR=${_docbook_xsl_dir} -DPYTHON_EXECUTABLE=/usr/bin/python -DXSLTPROC=/usr/bin/xsltproc -DGZIP_EXECUTABLE=/bin/gzip -DTAGLIBCONFIG_EXECUTABLE=\$_buildprefix/bin/taglib-config ../../kid3
EOF
  chmod +x kid3/build.sh
fi

elif test "$compiler" = "msvc"; then

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

echo "### Installing to root directory"

BUILDROOT=../libs-msvc
test -d $BUILDROOT || mkdir $BUILDROOT
for f in bin/*.tgz; do
  tar xzf $f -C $BUILDROOT
done

else

if test "$1" = "clean"; then
  for d in zlib-${zlib_version} libogg-${libogg_version} \
           libvorbis-${libvorbis_version} flac-${libflac_version} \
           id3lib-${id3lib_version} taglib-${taglib_version} \
           ${ffmpeg_dir} chromaprint-${chromaprint_version} \
           mp4v2-${mp4v2_version}; do
    test -d $d/inst && rm -rf $d/inst
  done
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
sed -i 's/^\(.*-Werror=missing-prototypes\)/#\1/' ./configure
AV_CONFIGURE_OPTIONS="--cross-prefix=${cross_host}- --arch=x86 --target-os=mingw32 --sysinclude=/usr/${cross_host}/include --extra-cflags=-march=i486"
elif test $kernel = "MINGW"; then
AV_CONFIGURE_OPTIONS="--extra-cflags=-march=i486"
if test $(uname) = "MSYS_NT-6.1"; then
AV_CONFIGURE_OPTIONS="$AV_CONFIGURE_OPTIONS --target-os=mingw32"
fi
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
fi

if test ! -d mp4v2-${mp4v2_version}/inst; then
echo "### Building mp4v2"

cd mp4v2-${mp4v2_version}/
if test $kernel = "MINGW" || test "$compiler" = "cross-mingw" ||
   test $kernel = "Darwin"; then
autoreconf -i
fi
test -f Makefile || CXXFLAGS="$CXXFLAGS -g -O2 -DMP4V2_USE_STATIC_LIB" ./configure --enable-shared=no --enable-static=yes --disable-gch $CONFIGURE_OPTIONS
mkdir -p inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/mp4v2-${mp4v2_version}.tgz usr
cd ../..
fi


echo "### Installing to root directory"

BUILDROOT=/
if test $kernel = "Linux"; then
  test -d buildroot || mkdir buildroot
  BUILDROOT=`pwd`/buildroot/
  # Static build can be tested from Linux in kid3 directory
  if ! test -d kid3; then
    mkdir kid3
    if test "$compiler" = "cross-mingw"; then
      cat >kid3/build.sh <<EOF
cmake $CMAKE_BUILD_OPTION -DCMAKE_TOOLCHAIN_FILE=$thisdir/source/mingw.cmake -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON -DWITH_MP4V2=ON -DCMAKE_CXX_FLAGS="-g -O2 -DMP4V2_USE_STATIC_LIB" -DDOCBOOK_XSL_DIR=${_docbook_xsl_dir} ../../kid3
EOF
      cat >kid3/make_package.sh <<EOF
#!/bin/bash
VERSION=\$(grep VERSION config.h | cut -d'"' -f2)
INSTDIR=kid3-\$VERSION-win32
QT_PREFIX=\$(sed "s/set(QT_PREFIX \(.*\))/\1/;q" ../source/mingw.cmake)
QT_BIN_DIR=\${QT_PREFIX}/bin
QT_TRANSLATIONS_DIR=\${QT_PREFIX}/translations

test -d \$INSTDIR && rm -rf \$INSTDIR
mkdir -p \$INSTDIR
make install/strip DESTDIR=\$(pwd)/\$INSTDIR
echo "### Ignore make error"

_plugin_qt_version=\$(grep "Created by.*Qt" src/plugins/musicbrainzimport/moc_musicbrainzimportplugin.cpp)
_plugin_qt_version=\${_plugin_qt_version##* \(Qt }
_plugin_qt_version=\${_plugin_qt_version%%\)*}
_plugin_qt_version_nr=\${_plugin_qt_version//./}
if test \$_plugin_qt_version_nr -gt ${qt_version//./}; then
  echo "Plugin Qt version \$_plugin_qt_version is larger than Qt version $qt_version."
  echo "Loading plugins will fail!"
  exit 1
fi

cp -f po/*.qm doc/*/kid3*.html \$INSTDIR

for f in Qt5Core.dll Qt5Network.dll Qt5Gui.dll Qt5Xml.dll Qt5Widgets.dll Qt5Multimedia.dll Qt5Qml.dll Qt5Quick.dll libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll; do
  cp \$QT_BIN_DIR/\$f \$INSTDIR
done

for f in po/*.qm; do
  l=\${f#*_};
  l=\${l%.qm};
  test -f \$QT_TRANSLATIONS_DIR/qtbase_\$l.qm && cp \$QT_TRANSLATIONS_DIR/qtbase_\$l.qm \$INSTDIR
done

rm -f \$INSTDIR.zip
7z a \$INSTDIR.zip \$INSTDIR
EOF
      chmod +x kid3/make_package.sh
    else
      taglib_config_version=$taglib_version
      taglib_config_version=${taglib_config_version%beta*}
      cat >kid3/build.sh <<EOF
BUILDPREFIX=\$(cd ..; pwd)/buildroot/usr/local
export PKG_CONFIG_PATH=\$BUILDPREFIX/lib/pkgconfig
# For a LINUX_SELF_CONTAINED build, change BUILD_SHARED_LIBS, WITH_QML to ON and add
# -G Ninja -DCMAKE_CXX_COMPILER=g++-4.8 -DCMAKE_C_COMPILER=gcc-4.8 -DQT_QMAKE_EXECUTABLE=/opt/Qt5.6.3/5.6.3/gcc_64/bin/qmake -DLINUX_SELF_CONTAINED=ON -DWITH_READLINE=OFF
cmake -DBUILD_SHARED_LIBS=OFF -DWITH_TAGLIB=OFF -DHAVE_TAGLIB=1 -DTAGLIB_LIBRARIES:STRING="-L\$BUILDPREFIX/lib -ltag -lz" -DTAGLIB_CFLAGS:STRING="-I\$BUILDPREFIX/include/taglib -I\$BUILDPREFIX/include -DTAGLIB_STATIC" -DTAGLIB_VERSION:STRING="${taglib_config_version}" -DWITH_QT5=ON -DWITH_QML=OFF -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL" -DCMAKE_INCLUDE_PATH=\$BUILDPREFIX/include -DCMAKE_LIBRARY_PATH=\$BUILDPREFIX/lib -DCMAKE_PROGRAM_PATH=\$BUILDPREFIX/bin -DWITH_FFMPEG=ON -DFFMPEG_ROOT=\$BUILDPREFIX -DWITH_MP4V2=ON $CMAKE_BUILD_OPTION -DWITH_GCC_PCH=OFF -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. -DWITH_LIBDIR=. -DWITH_PLUGINSDIR=./plugins ../../kid3
EOF
    fi
    chmod +x kid3/build.sh
  fi
fi
if test $kernel = "Darwin"; then
  tar_cmd="sudo tar xmozf"
else
  tar_cmd="tar xmzf"
fi

if test -n "$ZLIB_ROOT_PATH"; then
  ${tar_cmd} bin/zlib-${zlib_version}.tgz -C $BUILDROOT
fi
${tar_cmd} bin/libogg-${libogg_version}.tgz -C $BUILDROOT
${tar_cmd} bin/libvorbis-${libvorbis_version}.tgz -C $BUILDROOT
${tar_cmd} bin/flac-${libflac_version}.tgz -C $BUILDROOT
${tar_cmd} bin/id3lib-${id3lib_version}.tgz -C $BUILDROOT
${tar_cmd} bin/taglib-${taglib_version}.tgz -C $BUILDROOT
${tar_cmd} bin/${ffmpeg_dir}.tgz -C $BUILDROOT
${tar_cmd} bin/chromaprint-${chromaprint_version}.tgz -C $BUILDROOT
${tar_cmd} bin/mp4v2-${mp4v2_version}.tgz -C $BUILDROOT

fi

echo "### Built successfully"
