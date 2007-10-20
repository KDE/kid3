if HAVE_TAGLIB

noinst_LTLIBRARIES = libtaglibext.la

INCLUDES = $(TAGLIB_INCLUDES) -I$(top_srcdir)/kid3 $(all_includes)

libtaglibext_la_SOURCES = \
  urllinkframe.cpp unsynchronizedlyricsframe.cpp generalencapsulatedobjectframe.cpp

noinst_HEADERS = \
  urllinkframe.h unsynchronizedlyricsframe.h

endif
