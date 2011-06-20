set(TAGLIBEXT_LIBRARIES)
if (HAVE_TAGLIB)
  add_subdirectory(taglibext)
  set(TAGLIBEXT_LIBRARIES taglibext)
endif (HAVE_TAGLIB)

set(kid3_SRCS filelist.cpp fileproxymodel.cpp frame.cpp framelist.cpp
	modeliterator.cpp taggedfileiconprovider.cpp comboboxdelegate.cpp
	frametable.cpp frametablemodel.cpp genres.cpp id3form.cpp kid3.cpp main.cpp m4afile.cpp
	mp3file.cpp configdialog.cpp exportdialog.cpp formatconfig.cpp formatbox.cpp
	importdialog.cpp importparser.cpp generalconfig.cpp
	importconfig.cpp miscconfig.cpp freedbconfig.cpp
	freedbimporter.cpp rendirdialog.cpp dirlist.cpp dirproxymodel.cpp taggedfile.cpp
	musicbrainzdialog.cpp musicbrainzconfig.cpp musicbrainzclient.cpp
	numbertracksdialog.cpp oggfile.cpp vcedit.c flacfile.cpp
	commandstablemodel.cpp taglibfile.cpp serverimporterconfig.cpp
	serverimportdialog.cpp importclient.cpp serverimporter.cpp
	discogsimporter.cpp discogsconfig.cpp
	musicbrainzreleaseimporter.cpp externalprocess.cpp trackdata.cpp trackdatamodel.cpp
	stringlistedit.cpp tracktypeimporter.cpp filterconfig.cpp
	filterdialog.cpp filefilter.cpp expressionparser.cpp pictureframe.cpp
	formatreplacer.cpp httpclient.cpp downloaddialog.cpp picturelabel.cpp
	browsecoverartdialog.cpp configtable.cpp configtablemodel.cpp
	attributedata.cpp browserdialog.cpp imageviewer.cpp editframedialog.cpp
	editframefieldsdialog.cpp playlistdialog.cpp playlistconfig.cpp
	playlistcreator.cpp amazonimporter.cpp amazonconfig.cpp
	recentfilesmenu.cpp playtoolbar.cpp textimporter.cpp textimportdialog.cpp
	trackdatamatcher.cpp tagimportdialog.cpp)

if (HAVE_QTDBUS)
  set(kid3_SRCS ${kid3_SRCS} scriptinterface.cpp)
endif (HAVE_QTDBUS)

if (WITH_GCC_PCH)
  if (WITH_KDE)
    set(_includeRegex "#include <[Qkcp]")
  else (WITH_KDE)
    set(_includeRegex "#include <[Qcp]")
  endif (WITH_KDE)
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(_additionalFlags "-g -DQT_DEBUG -D_REENTRANT")
  else (CMAKE_BUILD_TYPE MATCHES Debug)
    set(_additionalFlags "-O3 -DQT_NO_DEBUG -D_REENTRANT -DNDEBUG")
  endif (CMAKE_BUILD_TYPE MATCHES Debug)
  ADD_PCH_HEADERFILE_RULE(${CMAKE_CURRENT_BINARY_DIR}/allsys.h
                          ${CMAKE_CURRENT_SOURCE_DIR} ${_includeRegex})
  ADD_PCH_RULE(allsys.h kid3_SRCS ${_additionalFlags})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winvalid-pch -include allsys.h")
endif (WITH_GCC_PCH)

if (WITH_KDE)

kde4_add_executable(kid3 ${kid3_SRCS})

target_link_libraries(kid3 ${TAGLIBEXT_LIBRARIES} ${BASIC_LIBRARIES}
	${ID3LIB_LIBRARIES} ${TAGLIB_LIBRARIES} ${VORBIS_LIBRARIES} ${FLAC_LIBRARIES}
	${TUNEPIMP_LIBRARIES} ${MP4V2_LIBRARIES})

install(TARGETS kid3 DESTINATION ${BIN_INSTALL_DIR})
install(FILES kid3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kid3ui.rc DESTINATION ${DATA_INSTALL_DIR}/kid3)
install(FILES net.sourceforge.Kid3.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})

else (WITH_KDE)

if (APPLE)
  # Icon and properties for .plist file
  set(MACOSX_BUNDLE_ICON_FILE kid3.icns) 
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.6")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge.kid3")
  set(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2003-2011 Urs Fleisch")
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/../macosx/kid3.icns
		PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(kid3_SRCS ${kid3_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/../macosx/kid3.icns)
elseif (WIN32)
  if (MINGW)
    # support for Windows resource files with MinGW
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif (MINGW)
  # add Windows resource file with icon
  set(kid3_SRCS ${kid3_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/../win32/kid3win.rc)
endif (APPLE)

if (UNIX AND NOT APPLE)
  # install desktop file and icons for Linux
  add_custom_command(OUTPUT kid3-qt.desktop
		COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/fixdesktop.pl
			<${CMAKE_CURRENT_SOURCE_DIR}/kid3.desktop >kid3-qt.desktop
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kid3.desktop
  )
  add_custom_target("desktop-file" ALL DEPENDS kid3-qt.desktop)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.desktop
		DESTINATION "${WITH_DATAROOTDIR}/applications")
	install(FILES hisc-app-kid3.svgz
					DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/scalable/apps"
          RENAME kid3-qt.svgz)
	install(FILES hi48-app-kid3.png
					DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/48x48/apps"
          RENAME kid3-qt.png)
	install(FILES hi32-app-kid3.png
					DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/32x32/apps"
          RENAME kid3-qt.png)
	install(FILES hi16-app-kid3.png
					DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/16x16/apps"
          RENAME kid3-qt.png)
endif (UNIX AND NOT APPLE)

# Run moc on Q_OBJECT header files
set(kid3_MOC_HDRS filelist.h frametable.h frametablemodel.h id3form.h kid3.h configdialog.h
	exportdialog.h formatbox.h importdialog.h rendirdialog.h
	musicbrainzdialog.h musicbrainzclient.h numbertracksdialog.h
	serverimportdialog.h importclient.h serverimporter.h externalprocess.h stringlistedit.h
	filterdialog.h filefilter.h httpclient.h downloaddialog.h
	browsecoverartdialog.h configtable.h browserdialog.h imageviewer.h
	editframedialog.h editframefieldsdialog.h playlistdialog.h recentfilesmenu.h
	playtoolbar.h fileproxymodel.h trackdatamodel.h textimportdialog.h
	tagimportdialog.h)
if (HAVE_QTDBUS)
  set(kid3_MOC_HDRS ${kid3_MOC_HDRS} scriptinterface.h)
endif (HAVE_QTDBUS)
qt4_wrap_cpp(kid3_GEN_MOC_HDRS ${kid3_MOC_HDRS})

add_definitions(${QT_DEFINITIONS})
set(kid3_RESOURCES kid3.qrc)
qt4_add_resources(kid3_GEN_RCS ${kid3_RESOURCES})
add_executable(${KID3_EXECUTABLE} MACOSX_BUNDLE WIN32 ${kid3_SRCS}
	${kid3_GEN_MOC_HDRS} ${kid3_GEN_RCS})

target_link_libraries(${KID3_EXECUTABLE} ${TAGLIBEXT_LIBRARIES}
	${BASIC_LIBRARIES} ${ID3LIB_LIBRARIES} ${TAGLIB_LIBRARIES} ${VORBIS_LIBRARIES}
	${FLAC_LIBRARIES} ${TUNEPIMP_LIBRARIES} ${MP4V2_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${KID3_EXECUTABLE}
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${WITH_BINDIR})

if (APPLE OR WIN32)

  # Put Qt libraries and plugins into installation package to create a standalone
  # bundle on Mac OS X and Windows
  set(plugin_dest_dir ${WITH_BINDIR})
  set(bundle_app "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_BINDIR}/${KID3_EXECUTABLE}")
  if (APPLE)
    set(plugin_dest_dir kid3.app/Contents/PlugIns)
    set(bundle_app "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/kid3.app")
  endif (APPLE)
  if (WIN32)
    set(bundle_app "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_BINDIR}/kid3.exe")
  endif (WIN32)
  string(REPLACE "/./" "/" bundle_app ${bundle_app})

  # Install Qt JPEG and Phonon plugins
  install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}
    FILES_MATCHING PATTERN "*jpeg${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "*jpeg4${CMAKE_SHARED_LIBRARY_SUFFIX}")

  if (HAVE_PHONON)
		install(DIRECTORY "${QT_PLUGINS_DIR}/phonon_backend"
			DESTINATION ${plugin_dest_dir}
      FILES_MATCHING
        PATTERN "*d4${CMAKE_SHARED_LIBRARY_SUFFIX}" EXCLUDE
        PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif (HAVE_PHONON)

  # Install and fix up all dependencies of the executable and the plugins.
  # On Mac OS X only keep the i386 code in the Qt libraries and plugins.
  install(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${bundle_app}\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
    if (APPLE)
      file(GLOB_RECURSE _qtlibs \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/kid3.app/Contents/Frameworks/*Qt*\")
      file(GLOB_RECURSE _plugins \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/kid3.app/Contents/PlugIns/*.dylib\")
      foreach (_universalBinary \${_plugins} \${_qtlibs})
        execute_process(COMMAND lipo \${_universalBinary} -output \${_universalBinary} -thin i386)
      endforeach (_universalBinary \${plugins})
    endif (APPLE)
    ")

endif (APPLE OR WIN32)

endif (WITH_KDE)
