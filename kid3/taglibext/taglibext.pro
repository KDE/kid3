# qmake subdirectory project file

!include(../../config.pri) {
  error("config.pri not found")
}

contains(CFG_LIBS, -ltag) {

  !isEmpty(CFG_EXTRA_DEFINES) {
    DEFINES += $$CFG_EXTRA_DEFINES
  }
  !isEmpty(CFG_EXTRA_INCLUDES) {
    INCLUDEPATH += $$CFG_EXTRA_INCLUDES
  }
  !isEmpty(TAGLIB_INCLUDES) {
    contains($$list($$[QT_VERSION]), 4.*) {
      INCLUDEPATH += $$replace(TAGLIB_INCLUDES, -I, "")
    } else {
      QMAKE_CXXFLAGS += $$TAGLIB_INCLUDES
    }
  }
  win32 {
    INCLUDEPATH += $$PWD\..
    QMAKE_CXXFLAGS += -I..
  }
  unix {
    INCLUDEPATH += $$PWD/.. ${OBJECTS_DIR}/..
  }

  CONFIG += $$CFG_CONFIG
  CONFIG += staticlib
  TEMPLATE = lib
  SOURCES = aac/aacfiletyperesolver.cpp mp2/mp2filetyperesolver.cpp
  HEADERS = aac/aacfiletyperesolver.h mp2/mp2filetyperesolver.h

  equals(TAGLIB_VERSION, 1.4) {
    SOURCES += urllinkframe.cpp unsynchronizedlyricsframe.cpp \
      generalencapsulatedobjectframe.cpp
    HEADERS += urllinkframe.h unsynchronizedlyricsframe.h \
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
  }

  TARGET = taglibext

}
