set(kid3_SRCS filelist.cpp filelistitem.cpp framelist.cpp genres.cpp id3form.cpp kid3.cpp main.cpp mp3file.cpp standardtags.cpp configdialog.cpp exportdialog.cpp formatconfig.cpp formatbox.cpp importdialog.cpp importselector.cpp importparser.cpp generalconfig.cpp importconfig.cpp miscconfig.cpp freedbdialog.cpp freedbconfig.cpp freedbclient.cpp rendirdialog.cpp dirlist.cpp taggedfile.cpp mp3framelist.cpp musicbrainzdialog.cpp musicbrainzconfig.cpp musicbrainzclient.cpp numbertracksdialog.cpp oggfile.cpp oggframelist.cpp vcedit.c flacfile.cpp flacframelist.cpp commandstable.cpp taglibfile.cpp taglibframelist.cpp urllinkframe.cpp unsynchronizedlyricsframe.cpp importsourceconfig.cpp importsourcedialog.cpp importsourceclient.cpp discogsdialog.cpp discogsclient.cpp discogsconfig.cpp musicbrainzreleasedialog.cpp musicbrainzreleaseclient.cpp externalprocess.cpp importtrackdata.cpp stringlistedit.cpp tracktypedialog.cpp tracktypeclient.cpp)

kde4_add_executable(kid3 ${kid3_SRCS})

target_link_libraries(kid3 ${BASIC_LIBRARIES} ${ID3LIB_LIBRARIES} ${TAGLIB_LIBRARIES} ${VORBIS_LIBRARIES} ${FLAC_LIBRARIES} ${TUNEPIMP_LIBRARIES})

install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
install(FILES kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kid3ui.rc DESTINATION ${DATA_INSTALL_DIR}/kid3)

kde4_install_icons(${ICON_INSTALL_DIR})
