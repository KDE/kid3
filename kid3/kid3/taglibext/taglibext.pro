# qmake subdirectory project file

!include(../../config.pri) {
  error("config.pri not found")
}

contains(CFG_LIBS, -ltag) {

  win32 {
    INCLUDEPATH = %MSYSDIR%\local\include %MSYSDIR%\local\include\taglib $$PWD\..
    QMAKE_CXXFLAGS += -I..
  }
  unix {
    INCLUDEPATH = /usr/include/taglib $$PWD/.. ${OBJECTS_DIR}/..
  }

  CONFIG += $$CFG_CONFIG
  CONFIG += staticlib
  TEMPLATE = lib
  SOURCES = urllinkframe.cpp unsynchronizedlyricsframe.cpp \
    generalencapsulatedobjectframe.cpp
  HEADERS = urllinkframe.h unsynchronizedlyricsframe.h \
    generalencapsulatedobjectframe.h
  SOURCES += speex/speexfile.cpp speex/speexproperties.cpp \
    speex/taglib_speexfiletyperesolver.cpp
  HEADERS += speex/speexfile.h speex/speexproperties.h \
    speex/taglib_speexfiletyperesolver.h
  SOURCES += trueaudio/taglib_trueaudiofiletyperesolver.cpp \
    trueaudio/ttafile.cpp trueaudio/ttaproperties.cpp
  HEADERS += trueaudio/combinedtag.h \
    trueaudio/taglib_trueaudiofiletyperesolver.h trueaudio/ttafile.h \
    trueaudio/ttaproperties.h
  SOURCES += wavpack/taglib_wavpackfiletyperesolver.cpp wavpack/wvfile.cpp \
    wavpack/wvproperties.cpp
  HEADERS += wavpack/combinedtag.h wavpack/taglib_wavpackfiletyperesolver.h \
    wavpack/wvfile.h wavpack/wvproperties.h
  SOURCES += aac/aacfiletyperesolver.cpp mp2/mp2filetyperesolver.cpp
  HEADERS += aac/aacfiletyperesolver.h mp2/mp2filetyperesolver.h

  TARGET = taglibext

}
