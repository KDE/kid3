if HAVE_TAGLIB

noinst_LTLIBRARIES = libtaglibext.la

INCLUDES = $(TAGLIB_INCLUDES) -I$(top_srcdir)/kid3 $(all_includes)

libtaglibext_la_SOURCES = \
  urllinkframe.cpp unsynchronizedlyricsframe.cpp generalencapsulatedobjectframe.cpp \
  speex/speexfile.cpp speex/speexproperties.cpp speex/taglib_speexfiletyperesolver.cpp \
  trueaudio/taglib_trueaudiofiletyperesolver.cpp \
  trueaudio/ttafile.cpp trueaudio/ttaproperties.cpp \
  wavpack/taglib_wavpackfiletyperesolver.cpp wavpack/wvfile.cpp \
  wavpack/wvproperties.cpp \
  aac/aacfiletyperesolver.cpp mp2/mp2filetyperesolver.cpp

noinst_HEADERS = \
  urllinkframe.h unsynchronizedlyricsframe.h \
  speex/speexfile.h speex/speexproperties.h speex/taglib_speexfiletyperesolver.h \
  trueaudio/combinedtag.h \
  trueaudio/taglib_trueaudiofiletyperesolver.h trueaudio/ttafile.h \
  trueaudio/ttaproperties.h \
  wavpack/combinedtag.h wavpack/taglib_wavpackfiletyperesolver.h \
  wavpack/wvfile.h wavpack/wvproperties.h \
  aac/aacfiletyperesolver.h mp2/mp2filetyperesolver.h

endif
