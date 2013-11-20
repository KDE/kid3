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
# The source code for the libraries is downloaded from Debian and Ubuntu
# repositories. If the files are no longer available, use a later version,
# it should still work.
#
# buildlibs.sh will download, build and install zlib, libogg, libvorbis,
# flac, id3lib, taglib, libav, chromaprint. You are then ready to build Kid3
# from the win32 or macosx directories by starting buildkid3.bat (Windows) or
# buildkid3.sh (Mac).

# Exit if an error occurs
set -e

thisdir=$(pwd)

kernel=$(uname)
test ${kernel:0:5} = "MINGW" && kernel="MINGW"

compiler="gcc"

qt_version=4.8.5
zlib_version=1.2.8
libogg_version=1.3.1
libav_version=0.8.7

# Uncomment for debug build
#ENABLE_DEBUG=--enable-debug
#CMAKE_BUILD_TYPE_DEBUG="-DCMAKE_BUILD_TYPE=Debug"

if ! which cmake >/dev/null; then
  echo cmake not found.
  return
  exit 1
fi

if test $kernel = "MINGW"; then
CMAKE_OPTIONS="-G \"MSYS Makefiles\" -DCMAKE_INSTALL_PREFIX=/usr/local"
elif test $kernel = "Darwin"; then
CMAKE_OPTIONS="-G \"Unix Makefiles\""
fi

if test "$compiler" = "cross-mingw"; then
CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_TOOLCHAIN_FILE=$thisdir/source/mingw.cmake"
CONFIGURE_OPTIONS="--host=i586-mingw32msvc"
fi

if test $kernel = "Darwin" && test $(uname -m) = "x86_64"; then
#ARCH=i386
if test "$ARCH" = "i386"; then
  # To build a 32-bit Mac OS X version of Kid3 use:
  # cmake -G "Unix Makefiles" -DCMAKE_CXX_FLAGS="-arch i386" -DCMAKE_C_FLAGS="-arch i386" -DCMAKE_EXE_LINKER_FLAGS="-arch i386" -DQT_QMAKE_EXECUTABLE=/usr/local/Trolltech/Qt-4.8.5-i386/bin/qmake -DCMAKE_BUILD_TYPE=Release -DWITH_FFMPEG=ON -DCMAKE_INSTALL_PREFIX= ../kid3
  # Building multiple architectures needs ARCH_FLAG="-arch i386 -arch x86_64",
  # CONFIGURE_OPTIONS="--disable-dependency-tracking", but it fails with libav.
  ARCH_FLAG="-arch i386"
else
  ARCH_FLAG="-Xarch_x86_64"
fi
CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_C_FLAGS=\"-O2 $ARCH_FLAG -mmacosx-version-min=10.5\" -DCMAKE_CXX_FLAGS=\"-O2 $ARCH_FLAG -mmacosx-version-min=10.5 -fvisibility=hidden -fvisibility-inlines-hidden\""
export CFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=10.5"
export CXXFLAGS="-O2 $ARCH_FLAG -mmacosx-version-min=10.5"
export LDFLAGS="$ARCH_FLAG -mmacosx-version-min=10.5"
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
    fi
    cd ..
  fi
}


# Download sources

test -d source || mkdir source
cd source

test -f flac_1.3.0-2.debian.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/flac/flac_1.3.0-2.debian.tar.gz
test -f flac_1.3.0.orig.tar.xz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/f/flac/flac_1.3.0.orig.tar.xz

test -f id3lib3.8.3_3.8.3-15.debian.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_3.8.3-15.debian.tar.gz
test -f id3lib3.8.3_3.8.3.orig.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_3.8.3.orig.tar.gz

test -f libogg_1.3.1-1.diff.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_1.3.1-1.diff.gz
test -f libogg_1.3.1.orig.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_1.3.1.orig.tar.gz

test -f libvorbis_1.3.2-1.3.diff.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_1.3.2-1.3.diff.gz
test -f libvorbis_1.3.2.orig.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_1.3.2.orig.tar.gz

test -f taglib-1.9.tar.gz ||
$DOWNLOAD http://taglib.github.io/releases/taglib-1.9.tar.gz

test -f zlib_1.2.8.dfsg-1.debian.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_1.2.8.dfsg-1.debian.tar.gz
test -f zlib_1.2.8.dfsg.orig.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_1.2.8.dfsg.orig.tar.gz

# With the new libav 9.5, some M4A fingerprints are not recognized,
# so we'll stick with the old.
if test "$libav_version" = "0.8.7"; then
test -f libav_0.8.7.orig.tar.xz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/liba/libav/libav_0.8.7.orig.tar.xz
test -f libav_0.8.7-1.debian.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/liba/libav/libav_0.8.7-1.debian.tar.gz
else
test -f libav_9.9.orig.tar.xz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/liba/libav/libav_9.9.orig.tar.xz
test -f libav_9.9-1.debian.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/liba/libav/libav_9.9-1.debian.tar.gz
fi

test -f chromaprint_0.7.orig.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/c/chromaprint/chromaprint_0.7.orig.tar.gz
test -f chromaprint_0.7-2.debian.tar.gz ||
$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/c/chromaprint/chromaprint_0.7-2.debian.tar.gz

#test -f mp4v2_1.9.1+svn479~dfsg0.orig.tar.bz2 ||
#$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/m/mp4v2/mp4v2_1.9.1+svn479~dfsg0.orig.tar.bz2
#test -f mp4v2_1.9.1+svn479~dfsg0-3.debian.tar.gz ||
#$DOWNLOAD http://ftp.de.debian.org/debian/pool/main/m/mp4v2/mp4v2_1.9.1+svn479~dfsg0-3.debian.tar.gz

# Create patch files

if test "$compiler" = "cross-mingw"; then
test -f mingw.cmake ||
cat >mingw.cmake <<EOF
set(QT_PREFIX /windows/Qt/$qt_version)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
set(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
set(CMAKE_RC_COMPILER i586-mingw32msvc-windres)
set(CMAKE_FIND_ROOT_PATH /usr/i586-mingw32msvc \${QT_PREFIX} $thisdir/buildroot/usr/local $thisdir/zlib-$zlib_version/inst/usr/local $thisdir/libav-$libav_version/inst/usr/local)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(QT_BINARY_DIR /usr/lib/i386-linux-gnu/qt4/bin)
set(QT_LIBRARY_DIR  \${QT_PREFIX}/lib)
set(QT_QTCORE_LIBRARY   \${QT_PREFIX}/lib/libQtCore4.a)
set(QT_QTCORE_INCLUDE_DIR \${QT_PREFIX}/include/QtCore)
set(QT_MKSPECS_DIR  \${QT_PREFIX}/mkspecs)
set(QT_MOC_EXECUTABLE  \${QT_BINARY_DIR}/moc)
set(QT_QMAKE_EXECUTABLE  \${QT_BINARY_DIR}/qmake)
set(QT_UIC_EXECUTABLE  \${QT_BINARY_DIR}/uic)
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

test -f id3lib-fix-utf16-stringlists.patch ||
cat >id3lib-fix-utf16-stringlists.patch <<EOF
diff -ru id3lib-3.8.3.orig/src/io_helpers.cpp id3lib-3.8.3/src/io_helpers.cpp
--- id3lib-3.8.3.orig/src/io_helpers.cpp	2012-08-26 19:52:21.523825799 +0200
+++ id3lib-3.8.3/src/io_helpers.cpp	2012-08-26 19:53:02.060028394 +0200
@@ -373,10 +373,17 @@
     //}
     // Right code
     unsigned char *pdata = (unsigned char *) data.c_str();
+    unicode_t lastCh = BOM;
     for (size_t i = 0; i < size; i += 2)
     {
       unicode_t ch = (pdata[i] << 8) | pdata[i+1];
+      if (lastCh == 0 && ch != BOM)
+      {
+        // Last character was NULL, so start next string with BOM.
+        writer.writeChars((const unsigned char*) &BOM, 2);
+      }
       writer.writeChars((const unsigned char*) &ch, 2);
+      lastCh = ch;
     }
     // End patch
   }

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

if test "$libav_version" = "0.8.7"; then
test -f libav_sws.patch ||
cat >libav_sws.patch <<"EOF"
--- cmdutils.c.org      2011-09-17 13:36:43.000000000 -0700
+++ cmdutils.c  2011-09-17 15:54:37.453134577 -0700
@@ -311,6 +311,11 @@
     const AVOption *oc, *of, *os;
     char opt_stripped[128];
     const char *p;
+// SPage: avoid sws_get_class failure
+#if !CONFIG_SWSCALE
+#   define sws_get_class(x)  0
+#endif
+
     const AVClass *cc = avcodec_get_class(), *fc = avformat_get_class(), *sc = sws_get_class();
 
     if (!(p = strchr(opt, ':')))
EOF
fi

cd ..


# Extract and patch sources

echo "### Extracting zlib"

if ! test -d zlib-1.2.8; then
tar xzf source/zlib_1.2.8.dfsg.orig.tar.gz
cd zlib-1.2.8/
tar xzf ../source/zlib_1.2.8.dfsg-1.debian.tar.gz || true
echo Can be ignored: Cannot create symlink to debian.series
for f in $(cat debian/patches/debian.series); do patch -p1 <debian/patches/$f; done
cd ..
fi

echo "### Extracting libogg"

if ! test -d libogg-1.3.1; then
tar xzf source/libogg_1.3.1.orig.tar.gz
cd libogg-1.3.1/
gunzip -c ../source/libogg_1.3.1-1.diff.gz | patch -p1
cd ..
fi

echo "### Extracting libvorbis"

if ! test -d libvorbis-1.3.2; then
tar xzf source/libvorbis_1.3.2.orig.tar.gz
cd libvorbis-1.3.2/
gunzip -c ../source/libvorbis_1.3.2-1.3.diff.gz | patch -p1
test -f win32/VS2008/libogg.vsprops.orig || mv win32/VS2008/libogg.vsprops win32/VS2008/libogg.vsprops.orig
sed "s/Value=\"1.1.4\"/Value=\"$libogg_version\"/" win32/VS2008/libogg.vsprops.orig >win32/VS2008/libogg.vsprops
cd ..
fi

echo "### Extracting libflac"

if ! test -d flac-1.3.0; then
unxz -c source/flac_1.3.0.orig.tar.xz | tar x
cd flac-1.3.0/
tar xzf ../source/flac_1.3.0-2.debian.tar.gz
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
patch -p1 <../source/flac_1.2.1_size_t_max_patch.diff
if test $kernel = "Darwin"; then
patch -p1 <../source/fink_flac.patch
if test "$ARCH" != "i386"; then
patch -p0 <patches/ltmain.sh.patch
fi
patch -p0 <patches/nasm.h.patch
fi
cd ..
fi

echo "### Extracting id3lib"

if ! test -d id3lib-3.8.3; then
tar xzf source/id3lib3.8.3_3.8.3.orig.tar.gz
cd id3lib-3.8.3/
tar xzf ../source/id3lib3.8.3_3.8.3-15.debian.tar.gz
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
patch -p1 <../source/id3lib-3.8.3_mingw.patch
patch -p1 <../source/id3lib-fix-utf16-stringlists.patch
test -f makefile.win32.orig || mv makefile.win32 makefile.win32.orig
sed 's/-W3 -WX -GX/-W3 -EHsc/; s/-MD -D "WIN32" -D "_DEBUG"/-MDd -D "WIN32" -D "_DEBUG"/' makefile.win32.orig >makefile.win32
cd ..
fi

echo "### Extracting taglib"

if ! test -d taglib-1.9; then
tar xzf source/taglib-1.9.tar.gz
cd taglib-1.9/
patch -p1 <../source/taglib-msvc.patch
cd ..
fi

echo "### Extracting libav"

if test "$libav_version" = "0.8.7"; then
if ! test -d libav-0.8.7; then
unxz -c source/libav_0.8.7.orig.tar.xz | tar x
cd libav-0.8.7/
tar xzf ../source/libav_0.8.7-1.debian.tar.gz
oldifs=$IFS
IFS='
'
for f in $(cat debian/patches/series); do
  if test "${f:0:1}" != "#"; then
    patch -p1 <debian/patches/$f
  fi
done
IFS=$oldifs
patch -p0 <../source/libav_sws.patch
cd ..
fi
else
if ! test -d libav-9.9; then
unxz -c source/libav_9.9.orig.tar.xz | tar x
cd libav-9.9/
tar xzf ../source/libav_9.9-1.debian.tar.gz
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
cd ..
fi
fi

echo "### Extracting chromaprint"

if ! test -d chromaprint-0.7; then
tar xzf source/chromaprint_0.7.orig.tar.gz
cd chromaprint-0.7/
tar xzf ../source/chromaprint_0.7-2.debian.tar.gz
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
cd ..
fi

#echo "### Extracting mp4v2"
#
#if ! test -d mp4v2-1.9.1+svn479~dfsg0; then
#tar xjf source/mp4v2_1.9.1+svn479~dfsg0.orig.tar.bz2
#cd mp4v2-1.9.1+svn479~dfsg0/
#tar xzf ../source/mp4v2_1.9.1+svn479~dfsg0-3.debian.tar.gz
#cd ..
#fi


# Build from sources

test -d bin || mkdir bin

if test "$compiler" = "msvc"; then

echo "### Building libogg"

cd libogg-1.3.1/
$COMSPEC /c "\"\"%VS90COMNTOOLS%vsvars32.bat\"\" && msbuild win32\VS2008\libogg_static.sln /p:Configuration=Debug;Platform=Win32"
$COMSPEC /c "\"\"%VS90COMNTOOLS%vsvars32.bat\"\" && msbuild win32\VS2008\libogg_static.sln /p:Configuration=Release;Platform=Win32"
mkdir -p inst/include/ogg inst/lib/Debug inst/lib/Release
cp win32/VS2008/Win32/Debug/libogg_static.lib inst/lib/Debug/
cp win32/VS2008/Win32/Release/libogg_static.lib inst/lib/Release/
cp include/ogg/*.h inst/include/ogg/
cd inst
tar czf ../../bin/libogg-1.3.1.tgz include lib
cd ../..

echo "### Building libvorbis"

cd libvorbis-1.3.2/
$COMSPEC /c "\"\"%VS90COMNTOOLS%vsvars32.bat\"\" && msbuild win32\VS2008\vorbis_static.sln /p:Configuration=Debug;Platform=Win32"
$COMSPEC /c "\"\"%VS90COMNTOOLS%vsvars32.bat\"\" && msbuild win32\VS2008\vorbis_static.sln /p:Configuration=Release;Platform=Win32"
mkdir -p inst/include/vorbis inst/lib/Debug inst/lib/Release
cp win32/VS2008/Win32/Debug/*.lib inst/lib/Debug/
cp win32/VS2008/Win32/Release/*.lib inst/lib/Release/
cp include/vorbis/*.h inst/include/vorbis/
cd inst
tar czf ../../bin/libvorbis-1.3.2.tgz include lib
cd ../..

echo "### Building id3lib"

cd id3lib-3.8.3/
test -f config.h || cp config.h.win32 config.h
$COMSPEC /c "\"\"%VS90COMNTOOLS%vsvars32.bat\"\" && nmake -f makefile.win32 DEBUG=1"
$COMSPEC /c "\"\"%VS90COMNTOOLS%vsvars32.bat\"\" && nmake -f makefile.win32"
mkdir -p inst/include inst/lib/Debug inst/lib/Release
cp -a include/id3* inst/include
cp id3libd.lib inst/lib/Debug/id3lib.lib
cp id3lib.lib inst/lib/Release/
cd inst
tar czf ../../bin/id3lib-3.8.3.tgz include lib
cd ../..

echo "### Building taglib"

cd taglib-1.9/
test -f taglib.sln || cmake -G "Visual Studio 9 2008" -DWITH_ASF=ON -DWITH_MP4=ON -DENABLE_STATIC=ON -DCMAKE_INSTALL_PREFIX=
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
tar czf ../../bin/taglib-1.9.tgz include lib
cd ../..

echo "### Installing to root directory"

BUILDROOT=../libs-msvc
test -d $BUILDROOT || mkdir $BUILDROOT
for f in bin/*.tgz; do
  tar xzf $f -C $BUILDROOT
done

else

echo "### Building zlib"

cd zlib-1.2.8/
if test $kernel = "MINGW"; then
make -f win32/Makefile.gcc
make install -f win32/Makefile.gcc INCLUDE_PATH=`pwd`/inst/usr/local/include LIBRARY_PATH=`pwd`/inst/usr/local/lib BINARY_PATH=`pwd`/inst/usr/local/bin
elif test "$compiler" = "cross-mingw"; then
make -f win32/Makefile.gcc PREFIX=i586-mingw32msvc-
make install -f win32/Makefile.gcc INCLUDE_PATH=`pwd`/inst/usr/local/include LIBRARY_PATH=`pwd`/inst/usr/local/lib BINARY_PATH=`pwd`/inst/usr/local/bin
else
CFLAGS="$CFLAGS -O3 -Wall -DNO_FSEEKO" ./configure --static
sed 's/LIBS=$(STATICLIB) $(SHAREDLIB) $(SHAREDLIBV)/LIBS=$(STATICLIB)/' Makefile >Makefile.inst
mkdir -p inst/usr/local
make install -f Makefile.inst prefix=`pwd`/inst/usr/local
fi
cd inst
tar czf ../../bin/zlib-1.2.8.tgz usr
cd ../..

echo "### Building libogg"

cd libogg-1.3.1/
test -f Makefile || ./configure --enable-shared=no --enable-static=yes $ENABLE_DEBUG $CONFIGURE_OPTIONS
make
mkdir -p inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/libogg-1.3.1.tgz usr
cd ../..

echo "### Building libvorbis"

cd libvorbis-1.3.2/
if test "$compiler" = "cross-mingw"; then
test -f Makefile || PKG_CONFIG= ./configure --enable-shared=no --enable-static=yes --with-ogg=$thisdir/libogg-$libogg_version/inst/usr/local $ENABLE_DEBUG $CONFIGURE_OPTIONS
else
test -f Makefile || ./configure --enable-shared=no --enable-static=yes --with-ogg=$thisdir/libogg-$libogg_version/inst/usr/local $ENABLE_DEBUG $CONFIGURE_OPTIONS
fi
make
mkdir -p inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/libvorbis-1.3.2.tgz usr
cd ../..

echo "### Building libflac"

cd flac-1.3.0/
configure_args="--enable-shared=no --enable-static=yes --with-ogg=$thisdir/libogg-$libogg_version/inst/usr/local $ENABLE_DEBUG $CONFIGURE_OPTIONS"
if test $kernel = "Darwin"; then
  configure_args="$configure_args --disable-asm-optimizations"
fi
test -f Makefile || ./configure $configure_args
make
mkdir -p inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/flac-1.3.0.tgz usr
cd ../..

echo "### Building id3lib"

cd id3lib-3.8.3/
autoconf
test -f Makefile || CPPFLAGS=-I/usr/local/include LDFLAGS=-L/usr/local/lib ./configure --enable-shared=no --enable-static=yes $ENABLE_DEBUG $CONFIGURE_OPTIONS
SED=sed make
mkdir -p inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/id3lib-3.8.3.tgz usr
cd ../..

echo "### Building taglib"

cd taglib-1.9/
test -f Makefile || eval cmake -DWITH_ASF=ON -DWITH_MP4=ON -DINCLUDE_DIRECTORIES=/usr/local/include -DLINK_DIRECTORIES=/usr/local/lib -DENABLE_STATIC=ON -DZLIB_ROOT=../zlib-$zlib_version/inst/usr/local $CMAKE_BUILD_TYPE_DEBUG $CMAKE_OPTIONS
make
mkdir -p inst
make install DESTDIR=`pwd`/inst
fixcmakeinst
cd inst
tar czf ../../bin/taglib-1.9.tgz usr
cd ../..

echo "### Building libav"

if test "$libav_version" = "0.8.7"; then
cd libav-0.8.7
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
AV_CONFIGURE_OPTIONS="--cross-prefix=i586-mingw32msvc- --arch=x86 --target-os=mingw32 --sysinclude=/usr/i586-mingw32msvc/include"
fi
./configure \
	--enable-memalign-hack \
	--disable-shared \
	--enable-static \
	--disable-debug \
	--disable-avdevice \
	--disable-avfilter \
	--disable-pthreads \
	--disable-swscale \
	--enable-ffmpeg \
	--disable-network \
	--disable-muxers \
	--disable-demuxers \
	--disable-sse \
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
tar czf ../../bin/libav-0.8.7.tgz usr
cd ../..
else
cd libav-9.9
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
AV_CONFIGURE_OPTIONS="--cross-prefix=i586-mingw32msvc- --arch=x86 --target-os=mingw32 --sysinclude=/usr/i586-mingw32msvc/include"
fi
./configure \
	--enable-memalign-hack \
	--disable-shared \
	--enable-static \
	--disable-debug \
	--disable-avdevice \
	--disable-avfilter \
	--disable-pthreads \
	--disable-swscale \
	--disable-network \
	--disable-muxers \
	--disable-demuxers \
	--disable-sse \
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
tar czf ../../bin/libav-9.9.tgz usr
cd ../..
fi

echo "### Building chromaprint"

# The zlib library path was added for MinGW-builds GCC 4.7.2.
cd chromaprint-0.7/
test -f Makefile || eval cmake -DBUILD_EXAMPLES=ON -DBUILD_SHARED_LIBS=OFF -DEXTRA_LIBS=\"-L$thisdir/zlib-$zlib_version/inst/usr/local/lib -lz\" -DFFMPEG_ROOT=$thisdir/libav-$libav_version/inst/usr/local $CMAKE_BUILD_TYPE_DEBUG $CMAKE_OPTIONS
mkdir -p inst
make install DESTDIR=`pwd`/inst
fixcmakeinst
cd inst
tar czf ../../bin/chromaprint-0.7.tgz usr
cd ../..

#echo "### Building mp4v2"
#
#cd mp4v2-1.9.1+svn479~dfsg0/
#test -f Makefile || ./configure --enable-shared=no --enable-static=yes --disable-gch $CONFIGURE_OPTIONS
#mkdir inst
#make install DESTDIR=`pwd`/inst
#cd inst
#tar czf ../../bin/mp4v2-1.9.1+svn479.tgz usr
#cd ../..


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
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=$thisdir/source/mingw.cmake -DCMAKE_INSTALL_PREFIX= -DWITH_FFMPEG=ON -DDOCBOOK_XSL_DIR=/usr/share/xml/docbook/stylesheet/nwalsh ../../kid3
EOF
      cat >kid3/make_package.sh <<"EOF"
#!/bin/sh
VERSION=$(grep VERSION config.h | cut -d'"' -f2)
INSTDIR=kid3-$VERSION-win32
QT_PREFIX=$(sed "s/set(QT_PREFIX \(.*\))/\1/;q" ../source/mingw.cmake)
QT_BIN_DIR=${QT_PREFIX}bin
QT_TRANSLATIONS_DIR=${QT_PREFIX}translations
MINGW_DIR=/windows/msys/1.0/mingw/bin

make install/strip DESTDIR=$(pwd)/$INSTDIR
echo "### Ignore make error"

cp -f po/*.qm doc/*/kid3*.html $INSTDIR

for f in QtCore4.dll QtNetwork4.dll QtGui4.dll QtXml4.dll phonon4.dll; do
  cp $QT_BIN_DIR/$f $INSTDIR
done

for f in libgcc_s_dw2-1.dll; do
  cp $MINGW_DIR/$f $INSTDIR
done

for f in po/*.qm; do
  l=${f#*_};
  l=${l%.qm};
  test -f $QT_TRANSLATIONS_DIR/qt_$l.qm && cp $QT_TRANSLATIONS_DIR/qt_$l.qm $INSTDIR
done

rm -f $INSTDIR.zip
7z a $INSTDIR.zip $INSTDIR
EOF
      chmod +x kid3/make_package.sh
    else
      cat >kid3/build.sh <<"EOF"
BUILDPREFIX=$(cd ..; pwd)/buildroot/usr/local
export PKG_CONFIG_PATH=$BUILDPREFIX/lib/pkgconfig
cmake -DWITH_TAGLIB=OFF -DHAVE_TAGLIB=1 -DTAGLIB_LIBRARIES:STRING="-L$BUILDPREFIX/lib -ltag" -DTAGLIB_CFLAGS:STRING="-I$BUILDPREFIX/include/taglib -I$BUILDPREFIX/include -DTAGLIB_STATIC" -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL" -DCMAKE_INCLUDE_PATH=$BUILDPREFIX/include -DCMAKE_LIBRARY_PATH=$BUILDPREFIX/lib -DCMAKE_PROGRAM_PATH=$BUILDPREFIX/bin -DWITH_FFMPEG=ON -DFFMPEG_ROOT=$BUILDPREFIX -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DWITH_GCC_PCH=OFF -DWITH_APPS="Qt;CLI" -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. ../../kid3
EOF
    fi
    chmod +x kid3/build.sh
  fi
elif test $kernel = "Darwin"; then
  sudo chmod go+w ${BUILDROOT}usr/local
fi

tar xzf bin/zlib-${zlib_version}.tgz -C $BUILDROOT
tar xzf bin/libogg-${libogg_version}.tgz -C $BUILDROOT
tar xzf bin/libvorbis-1.3.2.tgz -C $BUILDROOT
tar xzf bin/flac-1.3.0.tgz -C $BUILDROOT
tar xzf bin/id3lib-3.8.3.tgz -C $BUILDROOT
tar xzf bin/taglib-1.9.tgz -C $BUILDROOT
tar xzf bin/libav-${libav_version}.tgz -C $BUILDROOT
tar xzf bin/chromaprint-0.7.tgz -C $BUILDROOT
#tar xzf bin/mp4v2-1.9.1+svn479.tgz -C $BUILDROOT

if test $kernel = "Darwin"; then
  sudo chmod go-w ${BUILDROOT}usr/local
fi

fi

echo "### Built successfully"
