# qmake subdirectory project file

!include(../config.pri) {
  error("config.pri not found")
}

win32 {
  INCLUDEPATH = %MSYSDIR%\local\include %MSYSDIR%\local\include\taglib
  DEFINES += ID3LIB_LINKOPTION=1 FLAC__NO_DLL
  LIBS += -L%MSYSDIR%\local\lib
  RC_FILE = ../win32/kid3win.rc
}
unix {
  INCLUDEPATH = /usr/include/taglib $$PWD
}

TEMPLATE = app
QT = core gui network xml
LIBS += $$CFG_LIBS
CONFIG += $$CFG_CONFIG
PRECOMPILED_HEADER = $$CFG_PRECOMPILED_HEADER

contains(CFG_LIBS, -ltag) {
  win32 {
    release {
      TAGLIBEXT_LIB = taglibext\release\libtaglibext.a
    } else {
      TAGLIBEXT_LIB = taglibext\debug\libtaglibext.a
    }
  }
  unix {
    TAGLIBEXT_LIB = taglibext/libtaglibext.a
  }
  LIBS += $$TAGLIBEXT_LIB
  POST_TARGETDEPS += $$TAGLIBEXT_LIB
}

contains(CFG_LIBS, -lmp4v2) {
  win32: LIBS += -lwsock32
}

SOURCES = filelist.cpp filelistitem.cpp framelist.cpp frame.cpp frametable.cpp genres.cpp id3form.cpp kid3.cpp main.cpp m4afile.cpp mp3file.cpp standardtags.cpp configdialog.cpp  exportdialog.cpp formatconfig.cpp formatbox.cpp importdialog.cpp importselector.cpp importparser.cpp generalconfig.cpp importconfig.cpp miscconfig.cpp freedbdialog.cpp freedbconfig.cpp freedbclient.cpp rendirdialog.cpp dirlist.cpp taggedfile.cpp musicbrainzdialog.cpp musicbrainzconfig.cpp musicbrainzclient.cpp numbertracksdialog.cpp oggfile.cpp vcedit.c flacfile.cpp commandstable.cpp taglibfile.cpp importsourceconfig.cpp importsourcedialog.cpp importsourceclient.cpp discogsdialog.cpp discogsclient.cpp discogsconfig.cpp musicbrainzreleasedialog.cpp musicbrainzreleaseclient.cpp externalprocess.cpp importtrackdata.cpp stringlistedit.cpp tracktypedialog.cpp tracktypeclient.cpp

HEADERS = configdialog.h exportdialog.h filelist.h filelistitem.h formatbox.h formatconfig.h frame.h framelist.h frametable.h freedbclient.h freedbconfig.h freedbdialog.h generalconfig.h genres.h id3form.h importconfig.h importdialog.h importparser.h importselector.h kid3.h miscconfig.h m4afile.h mp3file.h rendirdialog.h standardtags.h dirlist.h taggedfile.h musicbrainzclient.h musicbrainzconfig.h musicbrainzdialog.h numbertracksdialog.h oggfile.hpp vcedit.h flacfile.hpp commandstable.h taglibfile.h importsourceconfig.h importsourcedialog.h importsourceclient.h discogsdialog.h discogsclient.h discogsconfig.h musicbrainzreleasedialog.h musicbrainzreleaseclient.h qtcompatmac.h dirinfo.h externalprocess.h stringlistedit.h tracktypedialog.h tracktypeclient.h

unix:program.path = $$CFG_BINDIR $$CFG_DATAROOTDIR/applications $$CFG_DATAROOTDIR/icons/hicolor/16x16/apps $$CFG_DATAROOTDIR/icons/hicolor/32x32/apps $$CFG_DATAROOTDIR/icons/hicolor/48x48/apps $$CFG_DATAROOTDIR/icons/hicolor/scalable/apps
win32 {
  isEmpty(CFG_BINDIR) {
    program.path = \.
  } else {
    program.path = $$CFG_BINDIR
  }
}
contains($$list($$[QT_VERSION]), 4.*) {
  contains($$list($$[QT_VERSION]), 4.2.*) {
    unix:program.extra = $(INSTALL_PROGRAM) $(TARGET) $(INSTALL_ROOT)$$CFG_BINDIR/kid3-qt; \
    $(INSTALL_FILE) kid3-qt.desktop $(INSTALL_ROOT)$$CFG_DATAROOTDIR/applications/kid3-qt.desktop; \
    $(INSTALL_FILE) hi16-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/16x16/apps/kid3-qt.png; \
    $(INSTALL_FILE) hi32-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/32x32/apps/kid3-qt.png; \
    $(INSTALL_FILE) hi48-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/48x48/apps/kid3-qt.png; \
    $(INSTALL_FILE) hisc-app-kid3-qt.svgz $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/scalable/apps/kid3-qt.svgz
    win32:program.extra = $(INSTALL_PROGRAM) $(DESTDIR_TARGET) $(INSTALL_ROOT)$$CFG_BINDIR
  } else {
    unix:program.extra = $(COPY_FILE) $(TARGET) $(INSTALL_ROOT)$$CFG_BINDIR/kid3-qt; \
    $(INSTALL_FILE) kid3-qt.desktop $(INSTALL_ROOT)$$CFG_DATAROOTDIR/applications/kid3-qt.desktop; \
    $(INSTALL_FILE) hi16-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/16x16/apps/kid3-qt.png; \
    $(INSTALL_FILE) hi32-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/32x32/apps/kid3-qt.png; \
    $(INSTALL_FILE) hi48-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/48x48/apps/kid3-qt.png; \
    $(INSTALL_FILE) hisc-app-kid3-qt.svgz $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/scalable/apps/kid3-qt.svgz
    win32:program.extra = $(COPY_FILE) $(DESTDIR_TARGET) $(INSTALL_ROOT)$$CFG_BINDIR
  }
} else {
  program.extra = $(INSTALL_FILE) $(TARGET) $(INSTALL_ROOT)$$CFG_BINDIR/kid3-qt; \
  $(INSTALL_FILE) kid3-qt.desktop $(INSTALL_ROOT)$$CFG_DATAROOTDIR/applications/kid3-qt.desktop; \
  $(INSTALL_FILE) hi16-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/16x16/apps/kid3-qt.png; \
  $(INSTALL_FILE) hi32-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/32x32/apps/kid3-qt.png; \
  $(INSTALL_FILE) hi48-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/48x48/apps/kid3-qt.png; \
  $(INSTALL_FILE) hisc-app-kid3-qt.svgz $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/scalable/apps/kid3-qt.svgz
  MOC_DIR = .
}

INSTALLS += program
