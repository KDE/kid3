set(qt_SRCS
  mainqt.cpp
  platformtools.cpp
  kid3mainwindow.cpp
  recentfilesmenu.cpp
  browserdialog.cpp
  messagedialog.cpp
  configdialog.cpp
  mainwindowconfig.cpp
  shortcutsmodel.cpp
  shortcutsdelegate.cpp
)
set(qt_MOC_HDRS
  kid3mainwindow.h
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
      "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
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
  if (GZIP_PROGRAM)
    add_custom_command(OUTPUT kid3-qt.svg
      COMMAND ${GZIP_PROGRAM} -cd ${CMAKE_CURRENT_SOURCE_DIR}/../hisc-app-kid3.svgz >kid3-qt.svg
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../hisc-app-kid3.svgz
    )
    set(qt_SRCS ${qt_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.svg)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.svg
            DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/scalable/apps")
  else (GZIP_PROGRAM)
    install(FILES ../hisc-app-kid3.svgz
            DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/scalable/apps"
            RENAME kid3-qt.svgz)
  endif (GZIP_PROGRAM)
  install(FILES ../hi48-app-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/48x48/apps"
          RENAME kid3-qt.png)
  install(FILES ../hi32-app-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/32x32/apps"
          RENAME kid3-qt.png)
  install(FILES ../hi16-app-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/16x16/apps"
          RENAME kid3-qt.png)
endif (UNIX AND NOT APPLE)

add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
set(qt_RESOURCES kid3.qrc)
qt4_add_resources(qt_GEN_RCS ${qt_RESOURCES})
add_executable(${KID3_EXECUTABLE} MACOSX_BUNDLE WIN32
  ${qt_SRCS} ${qt_GEN_RCS})

target_link_libraries(${KID3_EXECUTABLE} kid3-gui)

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
    if (APPLE)
      file(GLOB_RECURSE _qtlibs \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/kid3.app/Contents/Frameworks/*Qt*\")
      file(GLOB_RECURSE _plugins \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/kid3.app/Contents/PlugIns/*.dylib\")
      foreach (_universalBinary \${_plugins} \${_qtlibs})
        execute_process(COMMAND lipo \${_universalBinary} -output \${_universalBinary} -thin i386)
      endforeach (_universalBinary \${plugins})
    endif (APPLE)
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
            DESTINATION ${WITH_BINDIR})
  else (NOT HAVE_QT5)
    if (EXISTS "${_qt5Dir}/../ported-x32/bin/libjpeg-8.dll")
      install(FILES "${_qt5Dir}/../ported-x32/bin/libjpeg-8.dll"
              DESTINATION ${WITH_BINDIR})
    endif ()
    string(TOUPPER ${CMAKE_BUILD_TYPE} _build_type_uc)
    get_target_property(_qt5xml_location Qt5::Xml IMPORTED_LOCATION_${_build_type_uc})
    install(FILES ${_qt5xml_location}
            DESTINATION ${WITH_BINDIR})
  endif (NOT HAVE_QT5)
endif (WIN32)
