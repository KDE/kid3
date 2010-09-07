set(TAGLIBEXT_LIBRARIES)
if (HAVE_TAGLIB)
  add_subdirectory(taglibext)
  set(TAGLIBEXT_LIBRARIES taglibext)
endif (HAVE_TAGLIB)

set(kid3_SRCS filelist.cpp filelistitem.cpp frame.cpp framelist.cpp frametable.cpp genres.cpp id3form.cpp kid3.cpp main.cpp m4afile.cpp mp3file.cpp configdialog.cpp exportdialog.cpp formatconfig.cpp formatbox.cpp importdialog.cpp importselector.cpp importparser.cpp generalconfig.cpp importconfig.cpp miscconfig.cpp freedbdialog.cpp freedbconfig.cpp freedbclient.cpp rendirdialog.cpp dirlist.cpp taggedfile.cpp musicbrainzdialog.cpp musicbrainzconfig.cpp musicbrainzclient.cpp numbertracksdialog.cpp oggfile.cpp vcedit.c flacfile.cpp commandstable.cpp taglibfile.cpp importsourceconfig.cpp importsourcedialog.cpp importsourceclient.cpp discogsdialog.cpp discogsclient.cpp discogsconfig.cpp musicbrainzreleasedialog.cpp musicbrainzreleaseclient.cpp externalprocess.cpp importtrackdata.cpp stringlistedit.cpp tracktypedialog.cpp tracktypeclient.cpp filterconfig.cpp filterdialog.cpp filefilter.cpp expressionparser.cpp pictureframe.cpp formatreplacer.cpp httpclient.cpp downloaddialog.cpp picturelabel.cpp browsecoverartdialog.cpp configtable.cpp attributedata.cpp browserdialog.cpp imageviewer.cpp editframedialog.cpp editframefieldsdialog.cpp playlistdialog.cpp playlistconfig.cpp playlistcreator.cpp amazondialog.cpp amazonclient.cpp amazonconfig.cpp recentfilesmenu.cpp playtoolbar.cpp)

if (HAVE_QTDBUS)
  set(kid3_SRCS ${kid3_SRCS} scriptinterface.cpp)
endif (HAVE_QTDBUS)

kde4_add_executable(kid3 ${kid3_SRCS})

target_link_libraries(kid3 ${TAGLIBEXT_LIBRARIES} ${BASIC_LIBRARIES} ${ID3LIB_LIBRARIES} ${TAGLIB_LIBRARIES} ${VORBIS_LIBRARIES} ${FLAC_LIBRARIES} ${TUNEPIMP_LIBRARIES} ${MP4V2_LIBRARIES})

install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
install(FILES kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kid3ui.rc DESTINATION ${DATA_INSTALL_DIR}/kid3)
install(FILES net.sourceforge.Kid3.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})
