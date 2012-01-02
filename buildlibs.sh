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

kernel=$(uname)
test ${kernel:0:5} = "MINGW" && kernel="MINGW"

# Download sources

test -d source || mkdir source
cd source

test -f mp4v2-1.9.1.tar.bz2 ||
wget http://mp4v2.googlecode.com/files/mp4v2-1.9.1.tar.bz2

test -f flac_1.2.1-3.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/f/flac/flac_1.2.1-3.diff.gz
test -f flac_1.2.1.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/f/flac/flac_1.2.1.orig.tar.gz

test -f id3lib3.8.3_3.8.3-14.debian.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_3.8.3-14.debian.tar.gz
test -f id3lib3.8.3_3.8.3.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/i/id3lib3.8.3/id3lib3.8.3_3.8.3.orig.tar.gz

test -f libogg_1.2.2~dfsg-1.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_1.2.2~dfsg-1.diff.gz
test -f libogg_1.2.2~dfsg.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/libo/libogg/libogg_1.2.2~dfsg.orig.tar.gz

test -f libvorbis_1.3.2-1.diff.gz ||
wget http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_1.3.2-1.diff.gz
test -f libvorbis_1.3.2.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/libv/libvorbis/libvorbis_1.3.2.orig.tar.gz

test -f taglib_1.7-1.debian.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/t/taglib/taglib_1.7-1.debian.tar.gz
test -f taglib_1.7.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/t/taglib/taglib_1.7.orig.tar.gz

test -f zlib_1.2.5.dfsg-1.debian.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_1.2.5.dfsg-1.debian.tar.gz
test -f zlib_1.2.5.dfsg.orig.tar.gz ||
wget http://ftp.de.debian.org/debian/pool/main/z/zlib/zlib_1.2.5.dfsg.orig.tar.gz

# Create patch files

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

test -f taglib-mp4-uinttypes.patch ||
cat >taglib-mp4-uinttypes.patch <<"EOF"
diff --git a/taglib/mp4/mp4item.cpp b/taglib/mp4/mp4item.cpp
index 7dcf594..7d95079 100644
--- a/taglib/mp4/mp4item.cpp
+++ b/taglib/mp4/mp4item.cpp
@@ -45,6 +45,9 @@ public:
     bool m_bool;
     int m_int;
     IntPair m_intPair;
+    uchar m_byte;
+    uint m_uint;
+    long long m_longlong;
   };
   StringList m_stringList;
   MP4::CoverArtList m_coverArtList;
@@ -91,6 +94,24 @@ MP4::Item::Item(int value)
   d->m_int = value;
 }
 
+MP4::Item::Item(uchar value)
+{
+  d = new ItemPrivate;
+  d->m_byte = value;
+}
+
+MP4::Item::Item(uint value)
+{
+  d = new ItemPrivate;
+  d->m_uint = value;
+}
+
+MP4::Item::Item(long long value)
+{
+  d = new ItemPrivate;
+  d->m_longlong = value;
+}
+
 MP4::Item::Item(int value1, int value2)
 {
   d = new ItemPrivate;
@@ -122,6 +143,24 @@ MP4::Item::toInt() const
   return d->m_int;
 }
 
+uchar
+MP4::Item::toByte() const
+{
+  return d->m_byte;
+}
+
+TagLib::uint
+MP4::Item::toUInt() const
+{
+  return d->m_uint;
+}
+
+long long
+MP4::Item::toLongLong() const
+{
+  return d->m_longlong;
+}
+
 MP4::Item::IntPair
 MP4::Item::toIntPair() const
 {
diff --git a/taglib/mp4/mp4item.h b/taglib/mp4/mp4item.h
index 3158b4d..243a099 100644
--- a/taglib/mp4/mp4item.h
+++ b/taglib/mp4/mp4item.h
@@ -47,12 +47,18 @@ namespace TagLib {
       ~Item();
 
       Item(int value);
+      Item(uchar value);
+      Item(uint value);
+      Item(long long value);
       Item(bool value);
       Item(int first, int second);
       Item(const StringList &value);
       Item(const CoverArtList &value);
 
       int toInt() const;
+      uchar toByte() const;
+      uint toUInt() const;
+      long long toLongLong() const;
       bool toBool() const;
       IntPair toIntPair() const;
       StringList toStringList() const;
diff --git a/taglib/mp4/mp4tag.cpp b/taglib/mp4/mp4tag.cpp
index d7933db..a2f5d1f 100644
--- a/taglib/mp4/mp4tag.cpp
+++ b/taglib/mp4/mp4tag.cpp
@@ -68,12 +68,23 @@ MP4::Tag::Tag(TagLib::File *file, MP4::Atoms *atoms)
     else if(atom->name == "trkn" || atom->name == "disk") {
       parseIntPair(atom, file);
     }
-    else if(atom->name == "cpil" || atom->name == "pgap" || atom->name == "pcst") {
+    else if(atom->name == "cpil" || atom->name == "pgap" || atom->name == "pcst" ||
+            atom->name == "hdvd") {
       parseBool(atom, file);
     }
     else if(atom->name == "tmpo") {
       parseInt(atom, file);
     }
+    else if(atom->name == "tvsn" || atom->name == "tves" || atom->name == "cnID" ||
+            atom->name == "sfID" || atom->name == "atID" || atom->name == "geID") {
+      parseUInt(atom, file);
+    }
+    else if(atom->name == "plID") {
+      parseLongLong(atom, file);
+    }
+    else if(atom->name == "stik" || atom->name == "rtng" || atom->name == "akID") {
+      parseByte(atom, file);
+    }
     else if(atom->name == "gnre") {
       parseGnre(atom, file);
     }
@@ -138,6 +149,33 @@ MP4::Tag::parseInt(MP4::Atom *atom, TagLib::File *file)
 }
 
 void
+MP4::Tag::parseUInt(MP4::Atom *atom, TagLib::File *file)
+{
+  ByteVectorList data = parseData(atom, file);
+  if(data.size()) {
+    d->items.insert(atom->name, data[0].toUInt());
+  }
+}
+
+void
+MP4::Tag::parseLongLong(MP4::Atom *atom, TagLib::File *file)
+{
+  ByteVectorList data = parseData(atom, file);
+  if(data.size()) {
+    d->items.insert(atom->name, data[0].toLongLong());
+  }
+}
+
+void
+MP4::Tag::parseByte(MP4::Atom *atom, TagLib::File *file)
+{
+  ByteVectorList data = parseData(atom, file);
+  if(data.size()) {
+    d->items.insert(atom->name, (uchar)data[0].at(0));
+  }
+}
+
+void
 MP4::Tag::parseGnre(MP4::Atom *atom, TagLib::File *file)
 {
   ByteVectorList data = parseData(atom, file);
@@ -263,6 +301,30 @@ MP4::Tag::renderInt(const ByteVector &name, MP4::Item &item)
 }
 
 ByteVector
+MP4::Tag::renderUInt(const ByteVector &name, MP4::Item &item)
+{
+  ByteVectorList data;
+  data.append(ByteVector::fromUInt(item.toUInt()));
+  return renderData(name, 0x15, data);
+}
+
+ByteVector
+MP4::Tag::renderLongLong(const ByteVector &name, MP4::Item &item)
+{
+  ByteVectorList data;
+  data.append(ByteVector::fromLongLong(item.toLongLong()));
+  return renderData(name, 0x15, data);
+}
+
+ByteVector
+MP4::Tag::renderByte(const ByteVector &name, MP4::Item &item)
+{
+  ByteVectorList data;
+  data.append(ByteVector(1, item.toByte()));
+  return renderData(name, 0x15, data);
+}
+
+ByteVector
 MP4::Tag::renderIntPair(const ByteVector &name, MP4::Item &item)
 {
   ByteVectorList data;
@@ -339,12 +401,22 @@ MP4::Tag::save()
     else if(name == "disk") {
       data.append(renderIntPairNoTrailing(name.data(String::Latin1), i->second));
     }
-    else if(name == "cpil" || name == "pgap" || name == "pcst") {
+    else if(name == "cpil" || name == "pgap" || name == "pcst" || name == "hdvd") {
       data.append(renderBool(name.data(String::Latin1), i->second));
     }
     else if(name == "tmpo") {
       data.append(renderInt(name.data(String::Latin1), i->second));
     }
+    else if(name == "tvsn" || name == "tves" || name == "cnID" ||
+            name == "sfID" || name == "atID" || name == "geID") {
+      data.append(renderUInt(name.data(String::Latin1), i->second));
+    }
+    else if(name == "plID") {
+      data.append(renderLongLong(name.data(String::Latin1), i->second));
+    }
+    else if(name == "stik" || name == "rtng" || name == "akID") {
+      data.append(renderByte(name.data(String::Latin1), i->second));
+    }
     else if(name == "covr") {
       data.append(renderCovr(name.data(String::Latin1), i->second));
     }
diff --git a/taglib/mp4/mp4tag.h b/taglib/mp4/mp4tag.h
index 3e6d667..d0ff728 100644
--- a/taglib/mp4/mp4tag.h
+++ b/taglib/mp4/mp4tag.h
@@ -71,6 +71,9 @@ namespace TagLib {
         void parseText(Atom *atom, TagLib::File *file, int expectedFlags = 1);
         void parseFreeForm(Atom *atom, TagLib::File *file);
         void parseInt(Atom *atom, TagLib::File *file);
+        void parseByte(Atom *atom, TagLib::File *file);
+        void parseUInt(Atom *atom, TagLib::File *file);
+        void parseLongLong(Atom *atom, TagLib::File *file);
         void parseGnre(Atom *atom, TagLib::File *file);
         void parseIntPair(Atom *atom, TagLib::File *file);
         void parseBool(Atom *atom, TagLib::File *file);
@@ -83,6 +86,9 @@ namespace TagLib {
         TagLib::ByteVector renderFreeForm(const String &name, Item &item);
         TagLib::ByteVector renderBool(const ByteVector &name, Item &item);
         TagLib::ByteVector renderInt(const ByteVector &name, Item &item);
+        TagLib::ByteVector renderByte(const ByteVector &name, Item &item);
+        TagLib::ByteVector renderUInt(const ByteVector &name, Item &item);
+        TagLib::ByteVector renderLongLong(const ByteVector &name, Item &item);
         TagLib::ByteVector renderIntPair(const ByteVector &name, Item &item);
         TagLib::ByteVector renderIntPairNoTrailing(const ByteVector &name, Item &item);
         TagLib::ByteVector renderCovr(const ByteVector &name, Item &item);
EOF

test -f taglib-itunes-id3v22.patch ||
cat >taglib-itunes-id3v22.patch <<"EOF"
diff --git a/taglib/mpeg/id3v2/id3v2framefactory.cpp b/taglib/mpeg/id3v2/id3v2framefactory.cpp
index 40e454e..b6bc34a 100644
--- a/taglib/mpeg/id3v2/id3v2framefactory.cpp
+++ b/taglib/mpeg/id3v2/id3v2framefactory.cpp
@@ -310,6 +310,7 @@ bool FrameFactory::updateFrame(Frame::Header *header) const
     convertFrame("TBP", "TBPM", header);
     convertFrame("TCM", "TCOM", header);
     convertFrame("TCO", "TCON", header);
+    convertFrame("TCP", "TCMP", header);
     convertFrame("TCR", "TCOP", header);
     convertFrame("TDY", "TDLY", header);
     convertFrame("TEN", "TENC", header);
@@ -332,7 +333,12 @@ bool FrameFactory::updateFrame(Frame::Header *header) const
     convertFrame("TRC", "TSRC", header);
     convertFrame("TRD", "TDRC", header);
     convertFrame("TRK", "TRCK", header);
+    convertFrame("TS2", "TSO2", header);
+    convertFrame("TSA", "TSOA", header);
+    convertFrame("TSC", "TSOC", header);
+    convertFrame("TSP", "TSOP", header);
     convertFrame("TSS", "TSSE", header);
+    convertFrame("TST", "TSOT", header);
     convertFrame("TT1", "TIT1", header);
     convertFrame("TT2", "TIT2", header);
     convertFrame("TT3", "TIT3", header);
EOF

cd ..


# Extract and patch sources

# zlib

if ! test -d zlib-1.2.5.dfsg; then
tar xzf source/zlib_1.2.5.dfsg.orig.tar.gz
cd zlib-1.2.5.dfsg/
tar xzf ../source/zlib_1.2.5.dfsg-1.debian.tar.gz
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
cd ..
fi

# libogg

if ! test -d libogg-1.2.2~dfsg; then
tar xzf source/libogg_1.2.2~dfsg.orig.tar.gz
cd libogg-1.2.2~dfsg/
gunzip -c ../source/libogg_1.2.2~dfsg-1.diff.gz | patch -p1
cd ..
fi

# libvorbis

if ! test -d libvorbis-1.3.2.orig; then
tar xzf source/libvorbis_1.3.2.orig.tar.gz
cd libvorbis-1.3.2/
gunzip -c ../source/libvorbis_1.3.2-1.diff.gz | patch -p1
cd ..
fi

# libflac

if ! test -d flac-1.2.1; then
tar xzf source/flac_1.2.1.orig.tar.gz
cd flac-1.2.1/
gunzip -c ../source/flac_1.2.1-3.diff.gz | patch -p1
for f in debian/patches/*.dpatch; do patch -p1 <$f; done
patch -p1 <../source/flac_1.2.1_size_t_max_patch.diff
if test $kernel = "Darwin"; then
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
tar xzf ../source/id3lib3.8.3_3.8.3-14.debian.tar.gz
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
patch -p1 <../source/id3lib-3.8.3_mingw.patch
cd ..
fi

# taglib

if ! test -d taglib-1.7; then
tar xzf source/taglib_1.7.orig.tar.gz
cd taglib-1.7/
tar xzf ../source/taglib_1.7-1.debian.tar.gz
for f in $(cat debian/patches/series); do patch -p1 <debian/patches/$f; done
patch -p1 <../source/taglib-mp4-uinttypes.patch
patch -p1 <../source/taglib-itunes-id3v22.patch
cd ..
fi

# mp4v2

if ! test -d mp4v2-1.9.1; then
tar xjf source/mp4v2-1.9.1.tar.bz2
fi


# Build from sources

# Uncomment for debug build
#ENABLE_DEBUG=--enable-debug
#CMAKE_BUILD_TYPE_DEBUG="-DCMAKE_BUILD_TYPE=Debug"

if test $kernel = "MINGW"; then
CMAKE_OPTIONS="-G \"MSYS Makefiles\" -DCMAKE_INSTALL_PREFIX=/usr/local"
elif test $kernel = "Darwin"; then
CMAKE_OPTIONS="-G \"Unix Makefiles\""
fi

test -d bin || mkdir bin

# zlib

cd zlib-1.2.5.dfsg/
if test $kernel = "MINGW"; then
make -f win32/Makefile.gcc
make install -f win32/Makefile.gcc INCLUDE_PATH=`pwd`/inst/usr/local/include LIBRARY_PATH=`pwd`/inst/usr/local/lib
else
CFLAGS="-O3 -Wall -DNO_FSEEKO" ./configure --static
sed 's/LIBS=$(STATICLIB) $(SHAREDLIB) $(SHAREDLIBV)/LIBS=$(STATICLIB)/' Makefile >Makefile.inst
mkdir -p inst/usr/local
make install -f Makefile.inst prefix=`pwd`/inst/usr/local
fi
cd inst
tar czf ../../bin/zlib-1.2.5.tgz usr
cd ../..

# libogg

cd libogg-1.2.2~dfsg/
test -f Makefile || ./configure --enable-shared=no --enable-static=yes $ENABLE_DEBUG
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/libogg-1.2.2.tgz usr
cd ../..

# libvorbis

cd libvorbis-1.3.2/
test -f Makefile || ./configure --enable-shared=no --enable-static=yes --with-ogg=/usr/local $ENABLE_DEBUG
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/libvorbis-1.3.2.tgz usr
cd ../..

# libflac

cd flac-1.2.1/
configure_args="--enable-shared=no --enable-static=yes --with-ogg=/usr/local $ENABLE_DEBUG"
if test $kernel = "Darwin"; then
  configure_args="$configure_args --disable-asm-optimizations"
fi
test -f Makefile || ./configure $configure_args
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/flac-1.2.1.tgz usr
cd ../..

# id3lib

cd id3lib-3.8.3/
autoconf
test -f Makefile || CPPFLAGS=-I/usr/local/include LDFLAGS=-L/usr/local/lib ./configure --enable-shared=no --enable-static=yes $ENABLE_DEBUG
SED=sed make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/id3lib-3.8.3.tgz usr
cd ../..

# taglib

cd taglib-1.7/
test -f Makefile || eval cmake -DWITH_ASF=ON -DWITH_MP4=ON -DINCLUDE_DIRECTORIES=/usr/local/include -DLINK_DIRECTORIES=/usr/local/lib -DENABLE_STATIC=ON -DCMAKE_VERBOSE_MAKEFILE=ON $CMAKE_BUILD_TYPE_DEBUG $CMAKE_OPTIONS
make
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
if test $kernel = "MINGW"; then
  if test -d prg; then
    rm -rf usr
    mv prg/msys usr
    rmdir prg
  elif test -d msys; then
    rm -rf usr
    mv msys/1.0 usr
    rmdir msys
  fi
fi
tar czf ../../bin/taglib-1.7.tgz usr
cd ../..

# mp4v2

cd mp4v2-1.9.1/
# When getting from SVN with svn checkout http://mp4v2.googlecode.com/svn/trunk/ mp4v2-svn
# libtoolize
# aclocal
# automake --add-missing
# autoconf
test -f Makefile || ./configure --enable-shared=no --enable-static=yes --disable-gch $ENABLE_DEBUG
mkdir inst
make install DESTDIR=`pwd`/inst
cd inst
tar czf ../../bin/mp4v2-1.9.1.tgz usr
cd ../..


# Install to root directory

BUILDROOT=/
if test $kernel = "Linux"; then
  test -d buildroot || mkdir buildroot
  BUILDROOT=`pwd`/buildroot/
  # Static build can be tested from Linux in kid3 directory
  if ! test -d kid3; then
    mkdir kid3
    cat >kid3/build.sh <<"EOF"
BUILDPREFIX=$(cd ..; pwd)/buildroot/usr/local
cmake -DWITH_TAGLIB=OFF -DHAVE_TAGLIB=1 -DTAGLIB_LIBRARIES:STRING="-L$BUILDPREFIX/lib -ltag" -DTAGLIB_CFLAGS:STRING="-I$BUILDPREFIX/include/taglib -DTAGLIB_STATIC" -DCMAKE_CXX_FLAGS_DEBUG:STRING="-g -DID3LIB_LINKOPTION=1 -DFLAC__NO_DLL" -DCMAKE_INCLUDE_PATH=$BUILDPREFIX/include -DCMAKE_LIBRARY_PATH=$BUILDPREFIX/lib -DCMAKE_PROGRAM_PATH=$BUILDPREFIX/bin -DCMAKE_BUILD_TYPE=Debug -DWITH_GCC_PCH=OFF -DWITH_KDE=OFF -DWITH_TUNEPIMP=OFF -DCMAKE_INSTALL_PREFIX= -DWITH_BINDIR=. -DWITH_DATAROOTDIR=. -DWITH_DOCDIR=. -DWITH_TRANSLATIONSDIR=. ../../kid3
EOF
    chmod +x kid3/build.sh
  fi
elif test $kernel = "Darwin"; then
  sudo chmod go+w ${BUILDROOT}usr/local
fi

tar xzf bin/zlib-1.2.5.tgz -C $BUILDROOT
tar xzf bin/libogg-1.2.2.tgz -C $BUILDROOT
tar xzf bin/libvorbis-1.3.2.tgz -C $BUILDROOT
tar xzf bin/flac-1.2.1.tgz -C $BUILDROOT
tar xzf bin/id3lib-3.8.3.tgz -C $BUILDROOT
tar xzf bin/taglib-1.7.tgz -C $BUILDROOT
tar xzf bin/mp4v2-1.9.1.tgz -C $BUILDROOT

if test $kernel = "Darwin"; then
  sudo chmod go-w ${BUILDROOT}usr/local
fi
