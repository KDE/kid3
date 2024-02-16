add_executable(${KID3_EXECUTABLE} MACOSX_BUNDLE WIN32
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
qt_wrap_cpp(qt_GEN_MOC_SRCS
  kid3mainwindow.h
  kid3qtapplication.h
  recentfilesmenu.h
  browserdialog.h
  messagedialog.h
  configdialog.h
  shortcutsmodel.h
  shortcutsdelegate.h
  TARGET ${KID3_EXECUTABLE}
)
target_sources(${KID3_EXECUTABLE} PRIVATE ${qt_GEN_MOC_SRCS})

if(APPLE)
  # Icon and properties for .plist file
  set(MACOSX_BUNDLE_BUNDLE_NAME "Kid3")
  set(MACOSX_BUNDLE_BUNDLE_EXECUTABLE "kid3")
  set(MACOSX_BUNDLE_ICON_FILE kid3.icns)
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${CPACK_PACKAGE_VERSION})
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.kid3")
  set(MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2003-${RELEASE_YEAR} Urs Fleisch")
  set_source_files_properties(kid3.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  target_sources(${KID3_EXECUTABLE} PRIVATE kid3.icns)
  set_source_files_properties(qt.conf
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  target_sources(${KID3_EXECUTABLE} PRIVATE qt.conf)
elseif(WIN32)
  if(MINGW)
    # support for Windows resource files with MinGW
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif()
  # add Windows resource file with icon
  target_sources(${KID3_EXECUTABLE} PRIVATE kid3win.rc)
endif()

if(UNIX AND NOT APPLE)
  find_program(GZIP_PROGRAM gzip)
  # install desktop file and icons for Linux
  add_custom_command(OUTPUT org.kde.kid3-qt.desktop
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/fixdesktop.py
      <${CMAKE_CURRENT_SOURCE_DIR}/../org.kde.kid3.desktop >org.kde.kid3-qt.desktop
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../org.kde.kid3.desktop
  )
  add_custom_target("desktop-file" ALL DEPENDS org.kde.kid3-qt.desktop)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kid3-qt.desktop
    DESTINATION "${WITH_DATAROOTDIR}/applications")
  add_custom_command(OUTPUT org.kde.kid3-qt.appdata.xml
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/fixappdata.py
      ${CMAKE_CURRENT_SOURCE_DIR}/../org.kde.kid3.appdata.xml >org.kde.kid3-qt.appdata.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../org.kde.kid3.appdata.xml
  )
  add_custom_target("appdata-file" ALL DEPENDS org.kde.kid3-qt.appdata.xml)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kid3-qt.appdata.xml
    DESTINATION "${WITH_DATAROOTDIR}/metainfo")
  if(GZIP_PROGRAM)
    add_custom_command(OUTPUT kid3-qt.svg
      COMMAND ${GZIP_PROGRAM} -cdn ${CMAKE_CURRENT_SOURCE_DIR}/../sc-apps-kid3.svgz >kid3-qt.svg
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../sc-apps-kid3.svgz
    )
    target_sources(${KID3_EXECUTABLE} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.svg)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kid3-qt.svg
            DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/scalable/apps")
  else()
    install(FILES ../sc-apps-kid3.svgz
            DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/scalable/apps"
            RENAME kid3-qt.svgz)
  endif()
  install(FILES ../128-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/128x128/apps"
          RENAME kid3-qt.png)
  install(FILES ../64-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/64x64/apps"
          RENAME kid3-qt.png)
  install(FILES ../48-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/48x48/apps"
          RENAME kid3-qt.png)
  install(FILES ../32-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/32x32/apps"
          RENAME kid3-qt.png)
  install(FILES ../16-apps-kid3.png
          DESTINATION "${WITH_DATAROOTDIR}/icons/hicolor/16x16/apps"
          RENAME kid3-qt.png)
elseif(APPLE)
  install(FILES ../48-apps-kid3.png
          DESTINATION ${WITH_DATAROOTDIR}
          RENAME kid3.png)
endif()

qt_add_resources(qt_GEN_RCS kid3.qrc)
target_sources(${KID3_EXECUTABLE} PRIVATE
  ${qt_GEN_RCS}
)

target_link_libraries(${KID3_EXECUTABLE} kid3-gui)
if(NOT MSVC)
  target_link_libraries(${KID3_EXECUTABLE} -lstdc++)
endif()

if(APPLE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../../macosx/Info.plist.in
                 ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  set_target_properties(${KID3_EXECUTABLE} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
  if(CMAKE_CXX_COMPILER MATCHES "/osxcross/")
    # When cross compiling, CPack does not know that is should be APPLE.
    install(CODE "
  set(APPLE TRUE)
  ")
  endif()
endif()

install(TARGETS ${KID3_EXECUTABLE}
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${WITH_BINDIR})

if(LINUX_SELF_CONTAINED)
  # Change RPATH/RUNPATH of executable/library.
  macro(CHANGE_RPATH _rpath _file)
    install(CODE "
      execute_process(COMMAND chrpath -r ${_rpath} \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${_file}\"
                      RESULT_VARIABLE ret)
      if(NOT ret EQUAL 0)
        message(FATAL_ERROR \"CHANGE_RPATH(${_rpath} ${_file}) failed with \${ret}\")
      endif()
      ")
  endmacro()

  set_target_properties(${KID3_EXECUTABLE} PROPERTIES INSTALL_RPATH "\$ORIGIN")
endif()

if(WIN32)
  # CMAKE_INSTALL_PREFIX usually contains a drive letter which cannot be
  # prepended with some other prefix. Our build scripts set CMAKE_INSTALL_PREFIX
  # to an empty value, but this is overwritten by KDE craft.
  string(REGEX REPLACE "^[A-Za-z]:" "" destdir_prefix "${CMAKE_INSTALL_PREFIX}")
  set(destdir_prefix "\$ENV{DESTDIR}${destdir_prefix}")
else()
  set(destdir_prefix "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}")
endif()

if(APPLE OR WIN32 OR LINUX_SELF_CONTAINED)

  # Put Qt libraries and plugins into installation package to create a
  # standalone bundle on Mac OS X and Windows
  set(plugin_dest_dir ${WITH_BINDIR})
  set(bundle_app
    "${destdir_prefix}/${WITH_BINDIR}/${KID3_EXECUTABLE}")
  if(APPLE)
    set(plugin_dest_dir kid3.app/Contents/PlugIns)
    set(bundle_app "${destdir_prefix}/kid3.app")
  endif()
  if(WIN32)
    set(bundle_app
      "${destdir_prefix}/${WITH_BINDIR}/kid3.exe")
  endif()
  string(REPLACE "/./" "/" bundle_app ${bundle_app})

  # Install Qt JPEG, WebP and SVG icon plugins
  install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats"
    DESTINATION ${plugin_dest_dir}
    FILES_MATCHING PATTERN "*jpeg${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "*jpeg4${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "*webp${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "*.dSYM" EXCLUDE)
  install(DIRECTORY "${QT_PLUGINS_DIR}/iconengines"
    DESTINATION ${plugin_dest_dir}
    FILES_MATCHING PATTERN "*qsvgicon*${CMAKE_SHARED_LIBRARY_SUFFIX}"
                   PATTERN "*.dSYM" EXCLUDE)

  if(LINUX_SELF_CONTAINED)
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/imageformats/libqjpeg${CMAKE_SHARED_LIBRARY_SUFFIX}")
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/imageformats/libqwebp${CMAKE_SHARED_LIBRARY_SUFFIX}")
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/iconengines/libqsvgicon${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()
  if(APPLE)
    install(DIRECTORY "${QT_PLUGINS_DIR}/platforms"
      DESTINATION kid3.app/Contents/MacOS
      FILES_MATCHING PATTERN "libqcocoa${CMAKE_SHARED_LIBRARY_SUFFIX}"
                     PATTERN "*.dSYM" EXCLUDE)
    install(DIRECTORY "${QT_PLUGINS_DIR}/styles"
      DESTINATION ${plugin_dest_dir} OPTIONAL
      FILES_MATCHING PATTERN "libqmacstyle${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if(HAVE_QTMULTIMEDIA)
      if(QT_VERSION VERSION_GREATER_EQUAL 6.4)
        install(DIRECTORY "${QT_PLUGINS_DIR}/multimedia"
          DESTINATION ${plugin_dest_dir}
          FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
      else()
        install(DIRECTORY "${QT_PLUGINS_DIR}/mediaservice"
          DESTINATION ${plugin_dest_dir}
          FILES_MATCHING PATTERN "libqavfmediaplayer${CMAKE_SHARED_LIBRARY_SUFFIX}"
                         PATTERN "*.dSYM" EXCLUDE)
      endif()
    endif()
    if(QT_VERSION VERSION_GREATER_EQUAL 6.0)
      install(DIRECTORY "${QT_PLUGINS_DIR}/tls"
        DESTINATION ${plugin_dest_dir}
        FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()
  elseif(WIN32)
    install(DIRECTORY "${QT_PLUGINS_DIR}/platforms"
      DESTINATION ${plugin_dest_dir}
      FILES_MATCHING PATTERN "qwindows${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install(DIRECTORY "${QT_PLUGINS_DIR}/styles"
      DESTINATION ${plugin_dest_dir} OPTIONAL
      FILES_MATCHING PATTERN "qwindowsvistastyle${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if(HAVE_QTMULTIMEDIA)
      if(QT_VERSION VERSION_GREATER_EQUAL 6.4)
        install(DIRECTORY "${QT_PLUGINS_DIR}/multimedia"
          DESTINATION ${plugin_dest_dir}
          FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
      else()
        install(DIRECTORY "${QT_PLUGINS_DIR}/mediaservice"
          DESTINATION ${plugin_dest_dir}
          FILES_MATCHING PATTERN "dsengine${CMAKE_SHARED_LIBRARY_SUFFIX}")
      endif()
    endif()
    if(QT_VERSION VERSION_GREATER_EQUAL 6.0)
      install(DIRECTORY "${QT_PLUGINS_DIR}/tls"
        DESTINATION ${plugin_dest_dir}
        FILES_MATCHING PATTERN "*qopensslbackend${CMAKE_SHARED_LIBRARY_SUFFIX}"
                       PATTERN "*qschannelbackend${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()
  elseif(LINUX_SELF_CONTAINED)
    install(DIRECTORY "${QT_PLUGINS_DIR}/platforms"
      DESTINATION ${plugin_dest_dir}
      FILES_MATCHING PATTERN "libqxcb${CMAKE_SHARED_LIBRARY_SUFFIX}"
                     PATTERN "libqwayland-*${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install(DIRECTORY "${QT_PLUGINS_DIR}/wayland-graphics-integration-client"
      DESTINATION ${plugin_dest_dir}
      FILES_MATCHING PATTERN "libqt-plugin-wayland-egl${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install(DIRECTORY "${QT_PLUGINS_DIR}/wayland-shell-integration"
      DESTINATION ${plugin_dest_dir}
      FILES_MATCHING PATTERN "libxdg-shell${CMAKE_SHARED_LIBRARY_SUFFIX}")

    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/platforms/libqxcb${CMAKE_SHARED_LIBRARY_SUFFIX}")
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/platforms/libqwayland-generic${CMAKE_SHARED_LIBRARY_SUFFIX}")
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/platforms/libqwayland-egl${CMAKE_SHARED_LIBRARY_SUFFIX}")
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/wayland-graphics-integration-client/libqt-plugin-wayland-egl${CMAKE_SHARED_LIBRARY_SUFFIX}")
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/wayland-shell-integration/libxdg-shell${CMAKE_SHARED_LIBRARY_SUFFIX}")
    install(DIRECTORY "${QT_PLUGINS_DIR}/platforminputcontexts"
      DESTINATION ${plugin_dest_dir}
      FILES_MATCHING PATTERN "libcomposeplatforminputcontextplugin${CMAKE_SHARED_LIBRARY_SUFFIX}")
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/platforminputcontexts/libcomposeplatforminputcontextplugin${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if(QT_VERSION VERSION_LESS 6.0)
      if(HAVE_QTMULTIMEDIA)
        install(DIRECTORY "${QT_PLUGINS_DIR}/mediaservice"
          DESTINATION ${plugin_dest_dir}
          FILES_MATCHING PATTERN "libgstmediaplayer${CMAKE_SHARED_LIBRARY_SUFFIX}")
        CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/mediaservice/libgstmediaplayer${CMAKE_SHARED_LIBRARY_SUFFIX}")
      endif()
    else()
      install(DIRECTORY "${QT_PLUGINS_DIR}/tls"
        DESTINATION ${plugin_dest_dir}
        FILES_MATCHING PATTERN "libqopensslbackend${CMAKE_SHARED_LIBRARY_SUFFIX}")
      CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/tls/libqopensslbackend${CMAKE_SHARED_LIBRARY_SUFFIX}")
      if(QT_VERSION VERSION_GREATER_EQUAL 6.4)
        if(HAVE_QTMULTIMEDIA)
          install(DIRECTORY "${QT_PLUGINS_DIR}/multimedia"
            DESTINATION ${plugin_dest_dir}
            FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
          CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/multimedia/libgstreamermediaplugin${CMAKE_SHARED_LIBRARY_SUFFIX}")
        endif()
        CHANGE_RPATH("\$ORIGIN/../../../.." "${plugin_dest_dir}/plugins/imports/QtQml/WorkerScript/libworkerscriptplugin${CMAKE_SHARED_LIBRARY_SUFFIX}")
      endif()
    endif()
    install(DIRECTORY "${QT_PLUGINS_DIR}/xcbglintegrations"
      DESTINATION ${plugin_dest_dir}
      FILES_MATCHING PATTERN "libqxcb-glx-integration${CMAKE_SHARED_LIBRARY_SUFFIX}")
    CHANGE_RPATH("\$ORIGIN/.." "${plugin_dest_dir}/xcbglintegrations/libqxcb-glx-integration${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if(CMAKE_CROSSCOMPILING AND NOT APPLE)
    # Avoid error "/usr/bin/ldd failed: 1" from GetPrerequisites.cmake:800
    # when cross-packaging, this does not seem to be supported.
    # https://public.kitware.com/pipermail/cmake/2012-March/049497.html
    install(CODE "
    set(gp_tool DO_NOT_USE)
    ")
  endif()

  if(LINUX_SELF_CONTAINED)
    # CMake 3.11.1 gets the prerequisites type wrong if a plugin uses a
    # library which is in a parent directory but still inside the bundle.
    # Override the type from "other" to "local" in such cases.
    install(CODE "
    function(gp_resolved_file_type_override resolved_file type_var)
      if(\${\${type_var}} STREQUAL other)
        get_filename_component(bundle_app_path \"${bundle_app}\" PATH)
        string(LENGTH \"\${bundle_app_path}/\" bundle_app_length)
        string(LENGTH \"\${resolved_file}\" resolved_file_length)
        if(\${resolved_file_length} GREATER \${bundle_app_length})
          string(SUBSTRING \"\${resolved_file}\" 0 \${bundle_app_length} resolved_file_path)
          if(resolved_file_path STREQUAL \"\${bundle_app_path}/\")
            set(\${type_var} local PARENT_SCOPE)
          endif()
        endif()
      endif()
    endfunction()
    ")
  endif()

  # Install and fix up all dependencies of the executable and the plugins.
  # On Mac OS X only keep the i386 code in the Qt libraries and plugins.
  install(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"${destdir_prefix}/${plugin_dest_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    if(APPLE)
      file(GLOB_RECURSE QTPLATFORMPLUGINS
        \"${destdir_prefix}/kid3.app/Contents/MacOS/platforms/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
      set(QTPLUGINS \"\${QTPLUGINS};\${QTPLATFORMPLUGINS}\")
    endif()
    file(GLOB_RECURSE KID3PLUGINS
      \"${destdir_prefix}/${WITH_PLUGINSDIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    set(bundle_libs \"\${QTPLUGINS};\${KID3PLUGINS}\")
    if(APPLE AND BUILD_CLI_APP)
      set(bundle_libs \"\${bundle_libs};${destdir_prefix}/${WITH_LIBDIR}/kid3-cli\")
    endif()
    fixup_bundle(\"${bundle_app}\" \"\${bundle_libs}\" \"${QT_LIBRARY_DIR}\")
    ")

  if((CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo) AND
      CMAKE_STRIP)
    # Strip all shared libraries.
    install(CODE "
      file(GLOB_RECURSE _libsToStrip
        \"${destdir_prefix}/${WITH_BINDIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}*\")
      execute_process(COMMAND ${CMAKE_STRIP} \${_libsToStrip})
      ")
  endif()

endif()

if(WIN32)
  # Use reduced size ICU libraries, QTBUG-29828, QTBUG-38259.
  # A folder with reduced ICU libraries can be copied to the Qt bin directory.
  get_target_property(_qtBinDir Qt${QT_VERSION_MAJOR}::Core LOCATION)
  get_filename_component(_qtBinDir ${_qtBinDir} PATH)
  install(DIRECTORY "${_qtBinDir}/icu_reduced/"
          DESTINATION ${WITH_BINDIR} OPTIONAL)
  # Install missing DLLs.
  # Since Qt 5.12.4, OpenSSL 1.1.1 is supported, but the DLLs have different
  # names (libcrypto-1_1-x64.dll, libssl-1_1-x64.dll for 64-bit) and are not
  # shipped with Qt.
  if(Qt${QT_VERSION_MAJOR}Core_VERSION_STRING VERSION_LESS "5.12.4")
    file(GLOB _sslDlls "${_qtBinDir}/../../../Tools/mingw*/opt/bin/*eay32.dll")
  else()
    file(GLOB _sslDlls "${CMAKE_BINARY_DIR}/../buildroot/usr/local/ssl/*.dll")
  endif()
  install(FILES "${_qtBinDir}/Qt${QT_VERSION_MAJOR}Xml.dll" ${_sslDlls}
          DESTINATION ${WITH_BINDIR})
  if(QT_VERSION VERSION_GREATER_EQUAL 5.15)
    install(FILES "${_qtBinDir}/Qt${QT_VERSION_MAJOR}OpenGL.dll"
                  "${_qtBinDir}/Qt${QT_VERSION_MAJOR}QmlModels.dll"
                  "${_qtBinDir}/Qt${QT_VERSION_MAJOR}Quick.dll"
                  "${_qtBinDir}/Qt${QT_VERSION_MAJOR}QmlWorkerScript.dll"
            DESTINATION ${WITH_BINDIR})
  endif()
endif()

if(LINUX_SELF_CONTAINED)
  # Use reduced size ICU libraries.
  # A folder with reduced ICU libraries can be copied to the Qt bin directory.
  if(TARGET Qt${QT_VERSION_MAJOR}::qmake)
    get_target_property(_qtBinDir Qt${QT_VERSION_MAJOR}::qmake LOCATION)
  endif()
  get_filename_component(_qtBinDir ${_qtBinDir} PATH)
  string(REPLACE "/bin" "/lib" _qtLibDir "${_qtBinDir}")
  install(CODE "
    if(EXISTS \"${_qtLibDir}/icu_reduced\")
      file(GLOB _iculibs \"${destdir_prefix}/${WITH_BINDIR}/libicu*\")
      if(_iculibs)
        file(REMOVE \${_iculibs})
      endif()
    endif()
  ")
  install(DIRECTORY "${_qtLibDir}/icu_reduced/"
          DESTINATION ${WITH_BINDIR} OPTIONAL)
  # Qt searches for OpenSSL 1.0, which is not found on distributions
  # as new as Ubuntu 18.04. Install them in the application directory.
  install(FILES "${CMAKE_BINARY_DIR}/../buildroot/usr/local/ssl/libssl.so"
                "${CMAKE_BINARY_DIR}/../buildroot/usr/local/ssl/libcrypto.so"
          DESTINATION ${WITH_BINDIR})
endif()
