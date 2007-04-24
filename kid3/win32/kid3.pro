# Qt project file for Kid3

win32 {
  INCLUDEPATH = %MSYSDIR%\local\include %MSYSDIR%\local\include\taglib
  DEFINES += ID3LIB_LINKOPTION=1 FLAC__NO_DLL
  LIBS += -L%MSYSDIR%\local\lib
}
unix {
  INCLUDEPATH = /usr/include/taglib
}
CONFIG += release
RC_FILE = kid3win.rc
TEMPLATE = app
QT = core gui network xml qt3support
LIBS += -lid3 -logg -lvorbisfile -lvorbis -lFLAC++ -lFLAC -ltag

kid3_SOURCES = filelist.cpp filelistitem.cpp framelist.cpp genres.cpp id3form.cpp kid3.cpp main.cpp mp3file.cpp standardtags.cpp configdialog.h configdialog.cpp exportdialog.h exportdialog.cpp formatconfig.h formatconfig.cpp formatbox.h formatbox.cpp importdialog.h importdialog.cpp importselector.h importselector.cpp importparser.h importparser.cpp generalconfig.h generalconfig.cpp importconfig.h importconfig.cpp miscconfig.h miscconfig.cpp freedbdialog.h freedbdialog.cpp freedbconfig.h freedbconfig.cpp freedbclient.h freedbclient.cpp rendirdialog.h rendirdialog.cpp dirlist.cpp taggedfile.cpp mp3framelist.cpp musicbrainzdialog.h musicbrainzdialog.cpp musicbrainzconfig.h musicbrainzconfig.cpp musicbrainzclient.h musicbrainzclient.cpp numbertracksdialog.h numbertracksdialog.cpp oggfile.cpp oggframelist.cpp vcedit.c flacfile.cpp flacframelist.cpp commandstable.cpp taglibfile.cpp taglibframelist.cpp urllinkframe.cpp unsynchronizedlyricsframe.cpp importsourceconfig.cpp importsourcedialog.cpp importsourceclient.cpp discogsdialog.cpp discogsclient.cpp discogsconfig.cpp musicbrainzreleasedialog.cpp musicbrainzreleaseclient.cpp externalprocess.cpp importtrackdata.cpp stringlistedit.cpp

kid3_HEADERS = configdialog.h exportdialog.h filelist.h filelistitem.h formatbox.h formatconfig.h framelist.h freedbclient.h freedbconfig.h freedbdialog.h generalconfig.h genres.h id3form.h importconfig.h importdialog.h importparser.h importselector.h kid3.h miscconfig.h mp3file.h rendirdialog.h standardtags.h dirlist.h taggedfile.h mp3framelist.h musicbrainzclient.h musicbrainzconfig.h musicbrainzdialog.h numbertracksdialog.h oggfile.h oggframelist.h vcedit.h flacfile.h flacframelist.h commandstable.h taglibfile.h taglibframelist.h urllinkframe.h unsynchronizedlyricsframe.h importsourceconfig.h importsourcedialog.h importsourceclient.h discogsdialog.h discogsclient.h discogsconfig.h musicbrainzreleasedialog.h musicbrainzreleaseclient.h qtcompatmac.h dirinfo.h externalprocess.h stringlistedit.h

for(s, kid3_SOURCES): SOURCES += ../kid3/$${s}
for(h, kid3_HEADERS): HEADERS += ../kid3/$${h}

config_h.target = config.h
config_h.depends = config.mk
win32 {
  config_h.commands = %MSYSDIR%\bin\perl -ne \"print if (s/^([^$$LITERAL_HASH][^=\s]*)[\s=]+(.+)\$\$/$${LITERAL_HASH}define \1 \2/)\" config.mk >config.h
}
unix {
  config_h.commands = perl -ne "print if (s/^([^$$LITERAL_HASH][^=\s]*)[\s=]+(.+)\$\$/$${LITERAL_HASH}define \1 \2/)" config.mk >config.h
}
PRE_TARGETDEPS += $$config_h.target
QMAKE_EXTRA_TARGETS += config_h
