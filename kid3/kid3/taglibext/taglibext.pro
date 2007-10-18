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

  CONFIG += release staticlib
  TEMPLATE = lib
  SOURCES = urllinkframe.cpp unsynchronizedlyricsframe.cpp
  HEADERS = urllinkframe.h unsynchronizedlyricsframe.h
  TARGET = taglibext

}
