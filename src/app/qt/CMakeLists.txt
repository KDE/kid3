set(qt_SRCS
  mainqt.cpp
  platformtools.cpp
  kid3mainwindow.cpp
  kid3qtapplication.cpp
  recentfilesmenu.cpp
  browserdialog.cpp
  messagedialog.cpp
  configdialog.cpp
  shortcutsmodel.cpp
  shortcutsdelegate.cpp
)
set(qt_MOC_HDRS
  kid3mainwindow.h
  kid3qtapplication.h
  recentfilesmenu.h
  browserdialog.h
  messagedialog.h
  configdialog.h
  shortcutsmodel.h
  shortcutsdelegate.h
)
qt4_wrap_cpp(qt_GEN_MOC_SRCS ${qt_MOC_HDRS})
set(qt_SRCS ${qt_SRCS} ${qt_GEN_MOC_SRCS})

if (APPLE)
  # Icon and properties for .plist file
  set(MACOSX_BUNDLE_BUNDLE_NAME "Kid3")
  set(MACOSX_BUNDLE_BUNDLE_EXECUTABLE "kid3")
  set(MACOSX_BUNDLE_ICON_FILE kid3.icns)
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${CPACK_PACKAGE_VERSION})
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge.kid3")
  set(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2003-${RELEASE_YEAR} Urs Fleisch")
  set_source_files_properties(kid3.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(qt_SRCS ${qt_SRCS} kid3.icns)
  set_source_files_properties(qt.conf
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(qt_SRCS ${qt_SRCS} qt.conf)
elseif (WIN32)
  if (MINGW)
    # support for Windows resource files with MinGW
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif (MINGW)
  # add Windows resource file with icon
  set(qt_SRCS ${qt_SRCS} kid3win.rc)
endif (APPLE)

if (UNIX AND NOT APPLE)
  find_program(GZIP_PROGRAM gzip)
  # install desktop file and icons for Linux
  add_custom_command(OUTPUT kid3-qt.desktop
    COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/fixdesktop.pl
      <${CMAKE_CURRENT_SOURCE_DIR}/../kid3.desktop >kid3-qt.desktop
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../kid3.desktop
  )
  add_custom_target("desktop-file" ALL DEPENDS kid3-qt.desktop)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.desktop
    DESTINATION "${WITH_DATAROOTDIR}/applications")
  add_custom_command(OUTPUT kid3-qt.appdata.xml
    COMMAND ${PERL_EXECUTABLE} -pe "s/kid3\\.desktop/kid3-qt.desktop/"
      ${CMAKE_CURRENT_SOURCE_DIR}/../kid3.appdata.xml >kid3-qt.appdata.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../kid3.appdata.xml
  )
  add_custom_target("appdata-file" ALL DEPENDS kid3-qt.appdata.xml)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.appdata.xml
    DESTINATION "${WITH_DATAROOTDIR}/appdata")
  if (GZIP_PROGRAM)
    add_custom_command(OUTPUT kid3-qt.svg
      COMMAND ${GZIP_PROGRAM} -cdn ${CMAKE_CURRENT_SOURCE_DIR}/../hisc-apps-kid3.svgz >kid3-qt.svg
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../hisc-apps-kid3.svgz
    )
    set(qt_SRCS ${qt_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.svg)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.svg
            DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/scalable/apps")
  else (GZIP_PROGRAM)
    install(FILES ../hisc-apps-kid3.svgz
            DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/scalable/apps"
            RENAME kid3-qt.svgz)
  endif (GZIP_PROGRAM)
  install(FILES ../hi128-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/128x128/apps"
          RENAME kid3-qt.png)
  install(FILES ../hi64-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/64x64/apps"
          RENAME kid3-qt.png)
  install(FILES ../hi48-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/48x48/apps"
          RENAME kid3-qt.png)
  install(FILES ../hi32-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/32x32/apps"
          RENAME kid3-qt.png)
  install(FILES ../hi16-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/16x16/apps"
          RENAME kid3-qt.png)
elseif (APPLE)
  install(FILES ../hi48-apps-kid3.png
          DESTINATION ${WITH_DATAROOTDIR}
          RENAME kid3.png)
endif (UNIX AND NOT APPLE)

add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
set(qt_RESOURCES kid3.qrc)
qt4_add_resources(qt_GEN_RCS ${qt_RESOURCES})
add_executable(${KID3_EXECUTABLE} MACOSX_BUNDLE WIN32
  ${qt_SRCS} ${qt_GEN_RCS})

target_link_libraries(${KID3_EXECUTABLE} kid3-gui -lstdc++)

if (APPLE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../../macosx/Info.plist.in
                 ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  set_target_properties(${KID3_EXECUTABLE} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
endif (APPLE)

install(TARGETS ${KID3_EXECUTABLE}
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${WITH_BINDIR})

if (APPLE OR WIN32)

  # Put Qt libraries and plugins into installation package to create a
  # standalone bundle on Mac OS X and Windows
  set(plugin_dest_dir ${WITH_BINDIR})
  set(bundle_app
   "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_BINDIR}/${KID3_EXECUTABLE}")
  if (APPLE)
    set(plugin_dest_dir kid3.app/Contents/PlugIns)
    set(bundle_app "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/kid3.app")
  endif (APPLE)
  if (WIN32)
    set(bundle_app
      "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_BINDIR}/kid3.exe")
  endif (WIN32)
  string(REPLACE "/./" "/" bundle_app ${bundle_app})

  # Install Qt JPEG and Phonon plugins
  install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats"
    DESTINATION ${plugin_dest_dir}
    FILES_MATCHING PATTERN "*jpeg${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "*jpeg4${CMAKE_SHARED_LIBRARY_SUFFIX}")

  if (HAVE_QT5)
    if (APPLE)
      install(DIRECTORY "${QT_PLUGINS_DIR}/platforms"
        DESTINATION kid3.app/Contents/MacOS
        FILES_MATCHING PATTERN "libqcocoa${CMAKE_SHARED_LIBRARY_SUFFIX}")
      install(DIRECTORY "${QT_PLUGINS_DIR}/mediaservice"
        DESTINATION ${plugin_dest_dir}
        FILES_MATCHING PATTERN "libqavfmediaplayer${CMAKE_SHARED_LIBRARY_SUFFIX}")
    elseif (WIN32)
      install(DIRECTORY "${QT_PLUGINS_DIR}/platforms"
        DESTINATION ${plugin_dest_dir}
        FILES_MATCHING PATTERN "qwindows${CMAKE_SHARED_LIBRARY_SUFFIX}")
      install(DIRECTORY "${QT_PLUGINS_DIR}/mediaservice"
        DESTINATION ${plugin_dest_dir}
        FILES_MATCHING PATTERN "dsengine${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif (APPLE)
  endif (HAVE_QT5)

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
    if (APPLE)
      file(GLOB_RECURSE QTPLATFORMPLUGINS
        \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/kid3.app/Contents/MacOS/platforms/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
      set(QTPLUGINS \"\${QTPLUGINS};\${QTPLATFORMPLUGINS}\")
    endif (APPLE)
    file(GLOB_RECURSE KID3PLUGINS
      \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_PLUGINSDIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    set(bundle_libs \"\${QTPLUGINS};\${KID3PLUGINS}\")
    if (APPLE AND BUILD_CLI_APP)
      set(bundle_libs \"\${bundle_libs};\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${WITH_LIBDIR}/kid3-cli\")
    endif (APPLE AND BUILD_CLI_APP)
    fixup_bundle(\"${bundle_app}\" \"\${bundle_libs}\" \"${QT_LIBRARY_DIR}\")
    ")

endif (APPLE OR WIN32)

if (WIN32)
  if (NOT HAVE_QT5)
    # Unfortunately, the binutils which come with the gcc-4.4.0 in the MinGW
    # for Qt generate an executable which cannot be analyzed with dumpbin.
    # This happens when Kid3 is linked with libav (used with Chromaprint).
    # Therefore, the dependencies for kid3.exe are not added to the bundle.
    # This workaround adds the missing files.
    install(FILES "${QT_BINARY_DIR}/QtNetwork4.dll"
                  "${QT_BINARY_DIR}/QtXml4.dll"
                  "${QT_BINARY_DIR}/libwinpthread-1.dll"
                  "${QT_BINARY_DIR}/libiconv-2.dll"
            DESTINATION ${WITH_BINDIR})
  else (NOT HAVE_QT5)
    # Use reduced size ICU libraries, QTBUG-29828, QTBUG-38259.
    # A folder with reduced ICU libraries can be copied to the Qt bin directory.
    get_target_property(_qtBinDir Qt5::qmake LOCATION)
    get_filename_component(_qtBinDir ${_qtBinDir} PATH)
    install(DIRECTORY "${_qtBinDir}/icu_reduced/"
            DESTINATION ${WITH_BINDIR} OPTIONAL)
    # Install missing Qt5Xml.dll file.
    install(FILES "${_qtBinDir}/Qt5Xml.dll"
            DESTINATION ${WITH_BINDIR})
  endif (NOT HAVE_QT5)
endif (WIN32)
