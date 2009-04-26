#!/bin/sh
# This script can be used to build static libraries for the Windows and Mac
# versions of Kid3. Linux and BSD users do not need it because the libraries
# can be installed from their repositories.
#
# First you have to install the necessary tools:
#
# For Windows: MinGW/MSYS with development tools, Qt, xsltproc,
# html\docbook.xsl. Set the environment variables MSYSDIR, XSLTPROCDIR,
# DOCBOOKDIR to the install directories. They are used in win32/buildkit3.bat.
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
# flac, id3lib, taglib and mp4v2. You are then ready to build Kid3 from
# the win32 or macosx directories by starting buildkid3.bat (Windows) or
# buildkid3.sh (Mac). A binary package can be created using createpkg.bat,
# or createpkg.sh respectively.

# Download sources

test -d source || mkdir source
cd source

test -f faad2_2.0.0+cvs20040908+mp4v2+bmp-0ubuntu3.6.06.1.diff.gz ||
wget http://archive.ubuntu.com/ubuntu/pool/multiverse/f/faad2/faad2_2.0.0+cvs20040908+mp4v2+bmp-0ubuntu3.6.06.1.diff.gz
test -f faad2_2.0.0+cvs20040908+mp4v2+bmp.orig.tar.gz ||
wget http://archive.ubuntu.com/ubuntu/pool/multiverse/f/faad2/faad2_2.0.0+cvs20040908+mp4v2+bmp.orig.tar.gz

test -f flac_1.2.1-1.2.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/f/flac/flac_1.2.1-1.2.diff.gz
test -f flac_1.2.1.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/f/flac/flac_1.2.1.orig.tar.gz

test -f id3lib3.8.3_3.8.3-7.2.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_3.8.3-7.2.diff.gz
test -f id3lib3.8.3_3.8.3.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_3.8.3.orig.tar.gz

test -f libogg_1.1.3-2.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_1.1.3-2.diff.gz
test -f libogg_1.1.3.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_1.1.3.orig.tar.gz

test -f libvorbis_1.2.0.dfsg-3.1.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_1.2.0.dfsg-3.1.diff.gz
test -f libvorbis_1.2.0.dfsg.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_1.2.0.dfsg.orig.tar.gz

test -f taglib_1.5-6.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/t/taglib/taglib_1.5-6.diff.gz
test -f taglib_1.5.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/t/taglib/taglib_1.5.orig.tar.gz

test -f zlib_1.2.3-13.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_1.2.3-13.diff.gz
test -f zlib_1.2.3.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_1.2.3.orig.tar.gz


# Create patch files

test -f faad2_2.0.0_mac.patch ||
cat >faad2_2.0.0_mac.patch <<"EOF"
diff -ru faad2.orig/common/mp4v2/mp4util.h faad2/common/mp4v2/mp4util.h
--- faad2.orig/common/mp4v2/mp4util.h	2003-06-29 23:41:00.000000000 +0200
+++ faad2/common/mp4v2/mp4util.h	2009-04-18 07:37:27.000000000 +0200
@@ -22,6 +22,9 @@
 #ifndef __MP4_UTIL_INCLUDED__
 #define __MP4_UTIL_INCLUDED__
 #include <assert.h>
+#ifdef __APPLE__
+#include <sys/time.h>
+#endif
 
 #ifndef ASSERT
 #ifdef NDEBUG
diff -ru faad2.orig/common/mp4v2/systems.h faad2/common/mp4v2/systems.h
--- faad2.orig/common/mp4v2/systems.h	2009-04-22 22:38:40.000000000 +0200
+++ faad2/common/mp4v2/systems.h	2009-04-18 07:59:13.000000000 +0200
@@ -37,6 +37,14 @@
 #include <win32_ver.h>
 #endif
 #define NEED_SDL_VIDEO_IN_MAIN_THREAD
+#elif defined __APPLE__
+#ifndef PACKAGE
+#define PACKAGE "mpeg4ip"
+#endif
+#ifndef VERSION
+#define VERSION "0.9.8.6"
+#endif
+#define HAVE_STDINT_H
 #else
 #undef PACKAGE
 #undef VERSION
EOF

test -f faad2_2.0.0_mingw.patch ||
cat >faad2_2.0.0_mingw.patch <<"EOF"
diff -ru faad2.orig/common/mp4ff/mp4ff_int_types.h faad2/common/mp4ff/mp4ff_int_types.h
--- faad2.orig/common/mp4ff/mp4ff_int_types.h	Fri Nov  2 06:37:08 2007
+++ faad2/common/mp4ff/mp4ff_int_types.h	Sun Nov  4 12:10:52 2007
@@ -1,7 +1,7 @@
 #ifndef _MP4FF_INT_TYPES_H_
 #define _MP4FF_INT_TYPES_H_
 
-#ifdef _WIN32
+#if defined(_WIN32) && !defined(__MINGW32__)
 
 typedef char int8_t;
 typedef unsigned char uint8_t;
@@ -20,4 +20,4 @@
 #endif
 
 
-#endif
\ No newline at end of file
+#endif
diff -ru faad2.orig/common/mp4v2/systems.h faad2/common/mp4v2/systems.h
--- faad2.orig/common/mp4v2/systems.h	Fri Nov  2 06:40:12 2007
+++ faad2/common/mp4v2/systems.h	Sun Nov  4 12:11:32 2007
@@ -26,7 +26,16 @@
 #ifdef WIN32
 #define HAVE_IN_PORT_T
 #define HAVE_SOCKLEN_T
+#ifdef __MINGW32__
+#ifndef PACKAGE
+#define PACKAGE "mpeg4ip"
+#endif
+#ifndef VERSION
+#define VERSION "0.9.8.6"
+#endif
+#else
 #include <win32_ver.h>
+#endif
 #define NEED_SDL_VIDEO_IN_MAIN_THREAD
 #else
 #undef PACKAGE
@@ -49,6 +58,14 @@
 #include <time.h>
 #include <limits.h>
 
+#ifdef __MINGW32__
+#include <stdint.h>
+#include <ctype.h>
+typedef unsigned __int64 u_int64_t;
+typedef unsigned __int32 u_int32_t;
+typedef unsigned __int16 u_int16_t;
+typedef unsigned __int8 u_int8_t;
+#else
 typedef unsigned __int64 uint64_t;
 typedef unsigned __int32 uint32_t;
 typedef unsigned __int16 uint16_t;
@@ -64,6 +81,7 @@
 typedef unsigned short in_port_t;
 typedef int socklen_t;
 typedef int ssize_t;
+#endif
 #define snprintf _snprintf
 #define strncasecmp _strnicmp
 #define strcasecmp _stricmp
@@ -95,7 +113,9 @@
 }
 #endif
 
+#ifndef __MINGW32__
 #define PATH_MAX MAX_PATH
+#endif
 #define MAX_UINT64 -1
 #define LLD "%I64d"
 #define LLU "%I64u"
@@ -114,7 +134,7 @@
 #define LOG_INFO 6
 #define LOG_DEBUG 7
 
-#if     !__STDC__ && _INTEGRAL_MAX_BITS >= 64
+#if     (!__STDC__ || defined __MINGW32__) && _INTEGRAL_MAX_BITS >= 64
 #define VAR_TO_FPOS(fpos, var) (fpos) = (var)
 #define FPOS_TO_VAR(fpos, typed, var) (var) = (typed)(_FPOSOFF(fpos))
 #else
diff -ru faad2.orig/libfaad/common.h faad2/libfaad/common.h
--- faad2.orig/libfaad/common.h	Fri Nov  2 06:37:08 2007
+++ faad2/libfaad/common.h	Sun Nov  4 12:10:12 2007
@@ -303,6 +303,7 @@
     }
   #elif (defined(__i386__) && defined(__GNUC__))
     #define HAS_LRINTF
+#if !defined __MINGW32__ && !defined __APPLE__
     // from http://www.stereopsis.com/FPU.html
     static INLINE int lrintf(float f)
     {
@@ -314,6 +315,7 @@
             : "m" (f));
         return i;
     }
+#endif
   #endif
 
 
EOF

test -f faad2-mkinstalldirs.diff ||
cat >faad2-mkinstalldirs.diff <<"EOF"
diff -ruN faad2.orig/mkinstalldirs faad2/mkinstalldirs
--- faad2.orig/mkinstalldirs	Thu Jan  1 00:00:00 1970
+++ faad2/mkinstalldirs	Mon Mar  3 23:26:04 2008
@@ -0,0 +1,40 @@
+#! /bin/sh
+# mkinstalldirs --- make directory hierarchy
+# Author: Noah Friedman <friedman@prep.ai.mit.edu>
+# Created: 1993-05-16
+# Public domain
+
+# $Id: mkinstalldirs,v 1.1 1999/11/01 04:12:39 scott Exp $
+
+errstatus=0
+
+for file
+do
+   set fnord `echo ":$file" | sed -ne 's/^:\//#/;s/^://;s/\// /g;s/^#/\//;p'`
+   shift
+
+   pathcomp=
+   for d
+   do
+     pathcomp="$pathcomp$d"
+     case "$pathcomp" in
+       -* ) pathcomp=./$pathcomp ;;
+     esac
+
+     if test ! -d "$pathcomp"; then
+        echo "mkdir $pathcomp"
+
+        mkdir "$pathcomp" || lasterr=$?
+
+        if test ! -d "$pathcomp"; then
+  	  errstatus=$lasterr
+        fi
+     fi
+
+     pathcomp="$pathcomp/"
+   done
+done
+
+exit $errstatus
+
+# mkinstalldirs ends here
EOF

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
diff -ruN flac-1.2.1/src/plugin_xmms/Makefile.in flac-1.2.1.new/src/plugin_xmms/Makefile.in
--- flac-1.2.1/src/plugin_xmms/Makefile.in	2007-09-16 16:05:18.000000000 -0400
+++ flac-1.2.1.new/src/plugin_xmms/Makefile.in	2008-02-18 19:48:45.000000000 -0500
@@ -265,7 +265,6 @@
 	$(top_builddir)/src/share/replaygain_synthesis/libreplaygain_synthesis.la \
 	$(top_builddir)/src/share/utf8/libutf8.la \
 	$(top_builddir)/src/libFLAC/libFLAC.la \
-	-L$(top_builddir)/src/libFLAC/.libs \
 	@OGG_LIBS@ \
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
--- id3lib-3.8.3.orig/configure.in	Sun Mar  2 00:23:00 2003
+++ id3lib-3.8.3/configure.in	Thu Oct 11 08:55:26 2007
@@ -249,10 +249,10 @@
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
Only in id3lib-3.8.3: configure.in~
diff -ru id3lib-3.8.3.orig/include/id3/globals.h id3lib-3.8.3/include/id3/globals.h
--- id3lib-3.8.3.orig/include/id3/globals.h	Sun Mar  2 00:23:00 2003
+++ id3lib-3.8.3/include/id3/globals.h	Thu Oct 11 08:56:28 2007
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

test -f taglib-1.5-no_declspec.diff ||
cat >taglib-1.5-no_declspec.diff <<"EOF"
diff -ru taglib-1.5.orig/taglib/taglib_export.h taglib-1.5/taglib/taglib_export.h
--- taglib-1.5.orig/taglib/taglib_export.h	Mon Feb  4 15:14:46 2008
+++ taglib-1.5/taglib/taglib_export.h	Mon Mar  3 20:07:58 2008
@@ -26,7 +26,7 @@
 #ifndef TAGLIB_EXPORT_H
 #define TAGLIB_EXPORT_H
 
-#if defined(_WIN32) || defined(_WIN64)
+#if (defined(_WIN32) || defined(_WIN64)) && !defined __MINGW32__
 #ifdef MAKE_TAGLIB_LIB
 #define TAGLIB_EXPORT __declspec(dllexport)
 #else
EOF

cd ..


# Extract and patch sources

# zlib

if ! test -d zlib-1.2.3; then
tar xzf source/zlib_1.2.3.orig.tar.gz
cd zlib-1.2.3/
gunzip -c ../source/zlib_1.2.3-13.diff.gz | patch -p1
tar xzf upstream/tarballs/zlib-1.2.3.tar.gz
cd zlib-1.2.3/
for f in ../debian/patches/*; do patch -p1 <$f; done
cd ../..
fi

# libogg

if ! test -d libogg-1.1.3; then
tar xzf source/libogg_1.1.3.orig.tar.gz
cd libogg-1.1.3/
gunzip -c ../source/libogg_1.1.3-2.diff.gz | patch -p1
for f in debian/patches/*.diff; do patch -p0 <$f; done
cd ..
fi

# libvorbis

if ! test -d libvorbis-1.2.0; then
tar xzf source/libvorbis_1.2.0.dfsg.orig.tar.gz
cd libvorbis-1.2.0/
gunzip -c ../source/libvorbis_1.2.0.dfsg-3.1.diff.gz | patch -p1
for f in debian/patches/*.diff; do patch -p1 <$f; done
cd ..
fi

# libflac

if ! test -d flac-1.2.1; then
tar xzf source/flac_1.2.1.orig.tar.gz
cd flac-1.2.1/
gunzip -c ../source/flac_1.2.1-1.2.diff.gz | patch -p1
for f in debian/patches/*.dpatch; do patch -p1 <$f; done
patch -p1 <../source/flac_1.2.1_size_t_max_patch.diff
if test $(uname) = "Darwin"; then
patch -p1 <../source/fink_flac.patch
patch -p0 <patches/ltmain.sh.patch
patch -p0 <patches/nasm.h.patch
fi
cd ..
fi

# id3lib

if ! test -d id3lib-3.8.3; then
tar xzf source/id3lib3.8.3_3.8.3.orig.tar.gz
cd id3lib-3.8.3/
gunzip -c ../source/id3lib3.8.3_3.8.3-7.2.diff.gz | patch -p1
patch -p1 <../source/id3lib-3.8.3_mingw.patch
cd ..
fi

# taglib

if ! test -d taglib-1.5; then
tar xzf source/taglib_1.5.orig.tar.gz
cd taglib-1.5/
gunzip -c ../source/taglib_1.5-6.diff.gz | patch -p1
for f in debian/patches/general/*.diff; do patch -p1 <$f; done
patch -p1 <../source/taglib-1.5-no_declspec.diff
cd ..
fi

# mp4v2

if ! test -d faad2; then
tar xzf source/faad2_2.0.0+cvs20040908+mp4v2+bmp.orig.tar.gz
cd faad2/
gunzip -c ../source/faad2_2.0.0+cvs20040908+mp4v2+bmp-0ubuntu3.6.06.1.diff.gz | patch -p1
patch -p1 <debian/patches/01_systems.h.diff 
patch -p1 <debian/patches/04_mp4ff.h_fix.diff 
patch -p1 <debian/patches/06_pure_virtual_fix.diff 
patch -p1 <debian/patches/07_remove_static.diff 
patch -p0 <debian/patches/09_amd64.diff
patch -p1 <../source/faad2_2.0.0_mingw.patch
patch -p1 <../source/faad2-mkinstalldirs.diff
patch -p1 <../source/faad2_2.0.0_mac.patch
cd ..
fi


# Build from sources

test -d bin || mkdir bin

# zlib

cd zlib-1.2.3/zlib-1.2.3
test -f Makefile || ./configure
make
mkdir -p inst/usr/local
make install prefix=`pwd`/inst/usr/local
cd inst
tar czf ../../../bin/zlib-1.2.3.tgz usr
cd ../../..

# libogg

cd libogg-1.1.3/
test -f Makefile || ./configure --enable-shared=no --enable-static=yes
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/libogg-1.1.3.tgz usr
cd ../..

# libvorbis

cd libvorbis-1.2.0/
test -f Makefile || ./configure --enable-shared=no --enable-static=yes --with-ogg=/usr/local
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/libvorbis-1.2.0.tgz usr
cd ../..

# libflac

cd flac-1.2.1/
test -f Makefile || ./configure --enable-shared=no --enable-static=yes --with-ogg=/usr/local
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/flac-1.2.1.tgz usr
cd ../..

# id3lib

cd id3lib-3.8.3/
autoconf
test -f Makefile || CPPFLAGS=-I/usr/local/include LDFLAGS=-L/usr/local/lib ./configure --enable-shared=no --enable-static=yes
SED=sed make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/id3lib-3.8.3.tgz usr
cd ../..

# taglib

cd taglib-1.5/
test -f Makefile || CPPFLAGS=-I/usr/local/include LDFLAGS=-L/usr/local/lib ./configure --enable-shared=no --enable-static=yes
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/taglib-1.5.tgz usr
cd ../..

# mp4v2

cd faad2/
aclocal
automake
autoconf
if ! test -f configure.orig; then
  mv configure configure.orig
  sed 's/  PKG_CHECK_MODULES(BMP, bmp)/#  PKG_CHECK_MODULES(BMP, bmp)/' configure.orig >configure
  chmod +x configure
fi
test -f Makefile || ./configure --with-mp4v2 --without-xmms --without-bmp --enable-shared=no --enable-static=yes
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/faad2-2.0.0.tgz usr
cd ../..


# Install to root directory

if test $(uname) = "Darwin"; then
  sudo chmod go+w /usr/local
fi

tar xzf bin/zlib-1.2.3.tgz -C /
tar xzf bin/libogg-1.1.3.tgz -C /
tar xzf bin/libvorbis-1.2.0.tgz -C /
tar xzf bin/flac-1.2.1.tgz -C /
tar xzf bin/id3lib-3.8.3.tgz -C /
tar xzf bin/taglib-1.5.tgz -C /
tar xzf bin/faad2-2.0.0.tgz -C /

if test $(uname) = "Darwin"; then
  sudo chmod go-w /usr/local
fi
