# qmake subdirectory project file

!include(../config.pri) {
  error("config.pri not found")
}

!isEmpty(CFG_EXTRA_DEFINES) {
  DEFINES += $$CFG_EXTRA_DEFINES
}
!isEmpty(CFG_EXTRA_LIBS) {
  LIBS += $$CFG_EXTRA_LIBS
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
  RC_FILE = ../win32/kid3win.rc
}
unix {
  INCLUDEPATH += $$PWD
}

TEMPLATE = app
QT = core gui network xml
LIBS += $$CFG_LIBS
CONFIG += $$CFG_CONFIG
PRECOMPILED_HEADER = $$CFG_PRECOMPILED_HEADER

contains(CFG_LIBS, -ltag) {
  win32 {
    build_pass:CONFIG(debug, debug|release) {
      TAGLIBEXT_LIB = taglibext\debug\libtaglibext.a
    } else {
      TAGLIBEXT_LIB = taglibext\release\libtaglibext.a
    }
  }
  unix {
    TAGLIBEXT_LIB = taglibext/libtaglibext.a
  }
  LIBS += $$TAGLIBEXT_LIB
  POST_TARGETDEPS += $$TAGLIBEXT_LIB
}

!isEmpty(HAVE_PHONON) {
  QT += phonon
}

SOURCES = filelist.cpp filelistitem.cpp framelist.cpp frame.cpp frametable.cpp genres.cpp id3form.cpp kid3.cpp main.cpp m4afile.cpp mp3file.cpp configdialog.cpp  exportdialog.cpp formatconfig.cpp formatbox.cpp importdialog.cpp importselector.cpp importparser.cpp generalconfig.cpp importconfig.cpp miscconfig.cpp freedbdialog.cpp freedbconfig.cpp freedbclient.cpp rendirdialog.cpp dirlist.cpp taggedfile.cpp musicbrainzdialog.cpp musicbrainzconfig.cpp musicbrainzclient.cpp numbertracksdialog.cpp oggfile.cpp vcedit.c flacfile.cpp commandstable.cpp taglibfile.cpp importsourceconfig.cpp importsourcedialog.cpp importsourceclient.cpp discogsdialog.cpp discogsclient.cpp discogsconfig.cpp musicbrainzreleasedialog.cpp musicbrainzreleaseclient.cpp externalprocess.cpp importtrackdata.cpp stringlistedit.cpp tracktypedialog.cpp tracktypeclient.cpp filterconfig.cpp filterdialog.cpp filefilter.cpp expressionparser.cpp pictureframe.cpp formatreplacer.cpp httpclient.cpp downloaddialog.cpp picturelabel.cpp browsecoverartdialog.cpp configtable.cpp attributedata.cpp browserdialog.cpp imageviewer.cpp editframedialog.cpp editframefieldsdialog.cpp playlistdialog.cpp playlistconfig.cpp playlistcreator.cpp amazondialog.cpp amazonclient.cpp amazonconfig.cpp recentfilesmenu.cpp playtoolbar.cpp

HEADERS = configdialog.h exportdialog.h filelist.h filelistitem.h formatbox.h formatconfig.h frame.h framelist.h frametable.h freedbclient.h freedbconfig.h freedbdialog.h generalconfig.h genres.h id3form.h importconfig.h importdialog.h importparser.h importselector.h kid3.h miscconfig.h m4afile.h mp3file.h rendirdialog.h dirlist.h taggedfile.h musicbrainzclient.h musicbrainzconfig.h musicbrainzdialog.h numbertracksdialog.h oggfile.hpp vcedit.h flacfile.hpp commandstable.h taglibfile.h importsourceconfig.h importsourcedialog.h importsourceclient.h discogsdialog.h discogsclient.h discogsconfig.h musicbrainzreleasedialog.h musicbrainzreleaseclient.h qtcompatmac.h dirinfo.h externalprocess.h stringlistedit.h tracktypedialog.h tracktypeclient.h filterconfig.h filterdialog.h filefilter.h expressionparser.h pictureframe.h formatreplacer.h httpclient.h downloaddialog.h picturelabel.h browsecoverartdialog.h configtable.h attributedata.h browserdialog.h imageviewer.h editframedialog.h editframefieldsdialog.h playlistdialog.h playlistconfig.h playlistcreator.h amazondialog.h amazonclient.h amazonconfig.h recentfilesmenu.h playtoolbar.h

unix:program.path = $$CFG_BINDIR $$CFG_DATAROOTDIR/applications $$CFG_DATAROOTDIR/icons/hicolor/16x16/apps $$CFG_DATAROOTDIR/icons/hicolor/32x32/apps $$CFG_DATAROOTDIR/icons/hicolor/48x48/apps $$CFG_DATAROOTDIR/icons/hicolor/scalable/apps
win32 {
  isEmpty(CFG_BINDIR) {
    program.path = \.
  } else {
    program.path = $$CFG_BINDIR
  }
}
contains($$list($$[QT_VERSION]), 4.*) {
  contains($$list($$[QT_VERSION]), 4.1.*) {
    unix:program.extra = $(COPY_FILE) $(TARGET) $(INSTALL_ROOT)$$CFG_BINDIR/kid3-qt; \
    $(INSTALL_FILE) kid3-qt.desktop $(INSTALL_ROOT)$$CFG_DATAROOTDIR/applications/kid3-qt.desktop; \
    $(INSTALL_FILE) hi16-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/16x16/apps/kid3-qt.png; \
    $(INSTALL_FILE) hi32-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/32x32/apps/kid3-qt.png; \
    $(INSTALL_FILE) hi48-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/48x48/apps/kid3-qt.png; \
    $(INSTALL_FILE) hisc-app-kid3-qt.svgz $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/scalable/apps/kid3-qt.svgz
    win32:program.extra = $(COPY_FILE) $(DESTDIR_TARGET) $(INSTALL_ROOT)$$CFG_BINDIR
  } else {
    !isEmpty(HAVE_QTDBUS) {
      CONFIG += qdbus
      SOURCES += scriptinterface.cpp
      HEADERS += scriptinterface.h
    }
    unix:program.extra = $(INSTALL_PROGRAM) $(TARGET) $(INSTALL_ROOT)$$CFG_BINDIR/kid3-qt; \
    $(INSTALL_FILE) kid3-qt.desktop $(INSTALL_ROOT)$$CFG_DATAROOTDIR/applications/kid3-qt.desktop; \
    $(INSTALL_FILE) hi16-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/16x16/apps/kid3-qt.png; \
    $(INSTALL_FILE) hi32-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/32x32/apps/kid3-qt.png; \
    $(INSTALL_FILE) hi48-app-kid3-qt.png $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/48x48/apps/kid3-qt.png; \
    $(INSTALL_FILE) hisc-app-kid3-qt.svgz $(INSTALL_ROOT)$$CFG_DATAROOTDIR/icons/hicolor/scalable/apps/kid3-qt.svgz
    win32:program.extra = $(INSTALL_PROGRAM) $(DESTDIR_TARGET) $(INSTALL_ROOT)$$CFG_BINDIR
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

RESOURCES = kid3.qrc
