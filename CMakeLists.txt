# Build Kid3 using CMake
#
# Linux:
# cmake -DCMAKE_BUILD_TYPE=Release -DWITH_APPS=Qt -DCMAKE_INSTALL_PREFIX=/usr ../kid3
# make
# make install/strip DESTDIR=$(pwd)/inst # or
# cpack
#
# Windows:
# set INCLUDE=%MSYSDIR%\local\include
# set LIB=%MSYSDIR%\local\lib
# cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX= ..\kid3
# mingw32-make
# cpack
#
# Mac OS X:
# cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX= ../kid3
# make
# cpack

# These markers are to tell releaseme (used by KDE neon and others) to not inject translations
# SKIP_PO_INSTALL
# SKIP_POQML_INSTALL

cmake_minimum_required(VERSION 3.16)
project(kid3)
set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS ON CACHE BOOL "build shared libraries")
set(WITH_QML    ON CACHE BOOL "build with QML")
set(WITH_TAGLIB ON CACHE BOOL "build with TagLib")
set(WITH_MP4V2 OFF CACHE BOOL "build with mp4v2")
set(WITH_FFMPEG OFF CACHE BOOL "force use of FFmpeg for Chromaprint decoding")
set(WITH_CHROMAPRINT_FFMPEG OFF CACHE BOOL "link FFmpeg only for Chromaprint FFT")
set(WITH_GSTREAMER OFF CACHE BOOL "force use of GStreamer for Chromaprint decoding")
set(WITH_QAUDIODECODER OFF CACHE BOOL "force use of QAudioDecoder for Chromaprint decoding")
set(WITH_NO_MANCOMPRESS OFF CACHE BOOL "disable compressed manpages")
set(WITH_MULTIMEDIA ON CACHE BOOL "build with Qt Multimedia")
if(ANDROID)
  set(CMAKE_INSTALL_PREFIX "" CACHE PATH "install path prefix" FORCE)
  set(WITH_ID3LIB OFF CACHE BOOL "build with id3lib")
  set(WITH_VORBIS OFF CACHE BOOL "build with Ogg/Vorbis")
  set(WITH_FLAC OFF CACHE BOOL "build with FLAC")
  set(WITH_CHROMAPRINT OFF CACHE BOOL "build with Chromaprint")
else()
  set(WITH_ID3LIB ON CACHE BOOL "build with id3lib")
  set(WITH_VORBIS ON CACHE BOOL "build with Ogg/Vorbis")
  set(WITH_FLAC ON CACHE BOOL "build with FLAC")
  set(WITH_CHROMAPRINT ON CACHE BOOL "build with Chromaprint")
endif()
if(APPLE OR WIN32)
  set(WITH_DBUS OFF CACHE BOOL "build with QtDBus")
  set(WITH_APPS "Qt;CLI" CACHE STRING "build applications (Qt;CLI;Test)")
elseif(ANDROID)
  set(WITH_DBUS OFF CACHE BOOL "build with QtDBus")
  set(WITH_APPS "Qml" CACHE STRING "build applications (Qml;Qt;CLI;Test)")
else()
  set(WITH_DBUS ON CACHE BOOL "build with QtDBus")
  set(WITH_APPS "Qt;CLI;KDE" CACHE STRING "build applications (Qt;CLI;KDE;Test)")
endif()
if(WIN32)
  set(WITH_READLINE OFF CACHE BOOL "build with readline")
else()
  set(WITH_READLINE ON CACHE BOOL "build with readline")
endif()

if(DEFINED WITH_KDE)
  unset(WITH_KDE CACHE)
  message(FATAL_ERROR "The variable WITH_KDE is no longer used. The KDE and Qt "
                      "applications can now be built together using common "
                      "libraries. Use WITH_APPS with a value of \"KDE\" for "
                      "the KDE application, \"Qt\" for the Qt application, "
                      "\"CLI\" for the command line application or "
                      "\"Qt;CLI;KDE\" for all. Default is -DWITH_APPS="
                      "\"${WITH_APPS}\".")
endif()

if(WITH_APPS)
  string(TOLOWER "${WITH_APPS}" _apps_lower)
  foreach(_app ${_apps_lower})
    string(REPLACE "\"" "" _app ${_app})
    if(_app STREQUAL "qt")
      set(BUILD_QT_APP ON)
    elseif(_app STREQUAL "qml")
      set(BUILD_QML_APP ON)
      set(WITH_QML ON)
    elseif(_app STREQUAL "cli")
      set(BUILD_CLI_APP ON)
    elseif(_app STREQUAL "kde")
      set(BUILD_KDE_APP ON)
    elseif(_app STREQUAL "test")
      set(BUILD_TEST_APP ON)
    else()
      message(SEND_ERROR "Invalid value ${_app} in WITH_APPS. "
                         "Supported applications are \"Qt;KDE;Test\".")
    endif()
  endforeach()
endif()

if(NOT BUILD_SHARED_LIBS AND WITH_QML)
  message(WARNING "Switching off WITH_QML because of static build")
  set(WITH_QML OFF)
  set(BUILD_QML_APP OFF)
endif()


# Version information
set(CPACK_PACKAGE_VERSION_MAJOR 3)
set(CPACK_PACKAGE_VERSION_MINOR 9)
set(CPACK_PACKAGE_VERSION_PATCH 6)
set(KID3_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION ${KID3_VERSION})
#set(CPACK_PACKAGE_VERSION "git20240921")
set(RELEASE_YEAR 2024)

# Platform specific default installation paths
if(APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_DOCDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_TRANSLATIONSDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_QMLDIR_DEFAULT "kid3.app/Contents/Resources/qml")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT "kid3.app/Contents/MacOS")
  set(WITH_PLUGINSDIR_DEFAULT "kid3.app/Contents/PlugIns")
elseif(WIN32)
  set(WITH_DATAROOTDIR_DEFAULT ".")
  set(WITH_DOCDIR_DEFAULT ".")
  set(WITH_TRANSLATIONSDIR_DEFAULT ".")
  set(WITH_QMLDIR_DEFAULT "./qml")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT ".")
  set(WITH_PLUGINSDIR_DEFAULT "./plugins")
elseif(ANDROID)
  set(WITH_DATAROOTDIR_DEFAULT ".")
  set(WITH_DOCDIR_DEFAULT ".")
  set(WITH_TRANSLATIONSDIR_DEFAULT ":/translations")
  set(WITH_QMLDIR_DEFAULT ":/")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT ".")
  set(WITH_PLUGINSDIR_DEFAULT ".")
  set(QT_ANDROID_APP_VERSION ${KID3_VERSION})
  set(QT_ANDROID_APP_VERSION_CODE 30)
elseif(HAIKU)
  set(WITH_DATAROOTDIR_DEFAULT "data")
  set(WITH_DOCDIR_DEFAULT "data/doc/kid3-qt")
  set(WITH_TRANSLATIONSDIR_DEFAULT "data/kid3/translations")
  set(WITH_QMLDIR_DEFAULT "data/Qt6/qml")
  set(WITH_BINDIR_DEFAULT "bin")
  set(WITH_LIBDIR_DEFAULT "lib${LIB_SUFFIX}/kid3")
  set(WITH_PLUGINSDIR_DEFAULT "add-ons/Qt6/kid3")
else()
  set(WITH_DATAROOTDIR_DEFAULT "share")
  set(WITH_DOCDIR_DEFAULT "share/doc/kid3-qt")
  if(BUILD_KDE_APP OR BUILD_SHARED_LIBS)
    set(WITH_TRANSLATIONSDIR_DEFAULT "share/kid3/translations")
  else()
    set(WITH_TRANSLATIONSDIR_DEFAULT "share/kid3-qt/translations")
  endif()
  set(WITH_QMLDIR_DEFAULT "share/kid3/qml")
  set(WITH_BINDIR_DEFAULT "bin")
  set(WITH_LIBDIR_DEFAULT "lib${LIB_SUFFIX}/kid3")
  set(WITH_PLUGINSDIR_DEFAULT "${WITH_LIBDIR_DEFAULT}/plugins")
endif()

set(WITH_DATAROOTDIR ${WITH_DATAROOTDIR_DEFAULT} CACHE STRING
    "data root directory relative to CMAKE_INSTALL_PREFIX")
if(BUILD_KDE_APP)
  set(WITH_DBUSDIR "${WITH_DATAROOTDIR_DEFAULT}/dbus-1/interfaces" CACHE STRING
      "dbus directory relative to CMAKE_INSTALL_PREFIX")
endif()
set(WITH_DOCDIR ${WITH_DOCDIR_DEFAULT} CACHE STRING
    "documentation directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_TRANSLATIONSDIR ${WITH_TRANSLATIONSDIR_DEFAULT} CACHE STRING
    "translations directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_BINDIR ${WITH_BINDIR_DEFAULT} CACHE STRING
    "binary directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_LIBDIR ${WITH_LIBDIR_DEFAULT} CACHE STRING
    "library directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_PLUGINSDIR ${WITH_PLUGINSDIR_DEFAULT} CACHE STRING
    "plugin directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_MANDIR ${WITH_DATAROOTDIR}/man CACHE STRING
    "man documentation directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_QMLDIR ${WITH_QMLDIR_DEFAULT} CACHE STRING
    "QML directory relative to CMAKE_INSTALL_PREFIX")
if(CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "${CMAKE_INSTALL_PREFIX}/")
else()
  set(CMAKE_INSTALL_PREFIX_SLASH "")
endif()

if(APPLE)
  file(RELATIVE_PATH CFG_DATAROOTDIR "/kid3.app/Contents/MacOS" /${WITH_DATAROOTDIR})
  file(RELATIVE_PATH CFG_DOCDIR "/kid3.app/Contents/MacOS" /${WITH_DOCDIR})
  file(RELATIVE_PATH CFG_TRANSLATIONSDIR "/kid3.app/Contents/MacOS" /${WITH_TRANSLATIONSDIR})
  file(RELATIVE_PATH CFG_QMLDIR "/kid3.app/Contents/MacOS" /${WITH_QMLDIR})
  file(RELATIVE_PATH CFG_PLUGINSDIR "/kid3.app/Contents/MacOS" /${WITH_PLUGINSDIR})
elseif(WIN32)
  # When building with KDE craft, CMAKE_INSTALL_PREFIX would introduce paths inside the
  # CraftRoot. Remove the prefix and assume WITH_BINDIR to be the default ".".
  set(CFG_DATAROOTDIR ${WITH_DATAROOTDIR})
  set(CFG_DOCDIR ${WITH_DOCDIR})
  set(CFG_TRANSLATIONSDIR ${WITH_TRANSLATIONSDIR})
  set(CFG_QMLDIR ${WITH_QMLDIR})
  file(RELATIVE_PATH CFG_PLUGINSDIR /${WITH_BINDIR} /${WITH_PLUGINSDIR})
else()
  set(CFG_DATAROOTDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DATAROOTDIR})
  set(CFG_DOCDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DOCDIR})
  set(CFG_TRANSLATIONSDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_TRANSLATIONSDIR})
  set(CFG_QMLDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_QMLDIR})
  file(RELATIVE_PATH CFG_PLUGINSDIR /${WITH_BINDIR} /${WITH_PLUGINSDIR})
endif()
if(CFG_PLUGINSDIR STREQUAL "")
  set(CFG_PLUGINSDIR ".")
endif()
if(WITH_QMLDIR MATCHES "^:/")
  set(HAVE_QMLDIR_IN_QRC 1)
endif()
if(WITH_TRANSLATIONSDIR MATCHES "^:/")
  set(HAVE_TRANSLATIONSDIR_IN_QRC 1)
endif()
set(CFG_QMLSRCDIR "${kid3_SOURCE_DIR}/src/qml")

find_package(Threads)
include(CTest)
if(CMAKE_VERSION VERSION_LESS 3.12.0)
  set(Python_ADDITIONAL_VERSIONS 3)
  find_package(PythonInterp REQUIRED)
  # PythonInterp sets PYTHON_EXECUTABLE
else()
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
endif()
include (CheckCXXSourceCompiles)
include (CheckCXXCompilerFlag)
include (CheckLibraryExists)

# Find Qt
set(_qtDir)
string(REGEX MATCH "^(.*[Qq]t[/\\-]?([56]).*).bin.qmake.*" _qtDir "${QT_QMAKE_EXECUTABLE}")
if(_qtDir)
  set(_qtDir ${CMAKE_MATCH_1})
  set(_qtVersionMajor ${CMAKE_MATCH_2})
  set(QT_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}")
  set(Qt${_qtVersionMajor}Core_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Core")
  set(Qt${_qtVersionMajor}CoreTools_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}CoreTools")
  set(Qt${_qtVersionMajor}_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}")
  set(Qt${_qtVersionMajor}Gui_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Gui")
  set(Qt${_qtVersionMajor}GuiTools_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}GuiTools")
  set(Qt${_qtVersionMajor}Widgets_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Widgets")
  set(Qt${_qtVersionMajor}WidgetsTools_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}WidgetsTools")
  set(Qt${_qtVersionMajor}Network_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Network")
  set(Qt${_qtVersionMajor}Xml_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Xml")
  if(WITH_MULTIMEDIA)
    set(Qt${_qtVersionMajor}Multimedia_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Multimedia")
  endif()
  set(Qt${_qtVersionMajor}LinguistTools_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}LinguistTools")
  set(Qt${_qtVersionMajor}Test_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Test")
  if(WITH_DBUS)
    set(Qt${_qtVersionMajor}DBusTools_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}DBusTools")
    set(Qt${_qtVersionMajor}DBus_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}DBus")
  endif()
  if(WITH_QML)
    set(Qt${_qtVersionMajor}Qml_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Qml")
    set(Qt${_qtVersionMajor}QmlTools_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}QmlTools")
    set(Qt${_qtVersionMajor}Quick_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Quick")
  endif()
  if(BUILD_QML_APP)
    set(Qt${_qtVersionMajor}Svg_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}Svg")
    set(Qt${_qtVersionMajor}QuickControls2_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}QuickControls2")
  endif()
  if(ANDROID)
    if(_qtVersionMajor EQUAL 6)
      set(Qt${_qtVersionMajor}QmlIntegration_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}QmlIntegration")
      set(Qt${_qtVersionMajor}QmlModels_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}QmlModels")
      set(Qt${_qtVersionMajor}OpenGL_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}OpenGL")
      set(Qt${_qtVersionMajor}QuickTemplates2_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}QuickTemplates2")
    else()
      set(Qt${_qtVersionMajor}AndroidExtras_DIR "${_qtDir}/lib/cmake/Qt${_qtVersionMajor}AndroidExtras")
    endif()
  endif()
endif()
if(BUILD_WITH_QT6)
  find_package(QT NAMES Qt6 COMPONENTS Core REQUIRED)
elseif(BUILD_WITH_QT5)
  find_package(QT NAMES Qt5 COMPONENTS Core REQUIRED)
else()
  find_package(QT NAMES Qt5 Qt6 COMPONENTS Core REQUIRED)
endif()
if(QT_VERSION_MAJOR EQUAL 6 AND QT_VERSION_MINOR LESS 2)
  set(WITH_MULTIMEDIA OFF)
endif()
set(_qtComponents Core Gui Widgets Network Xml LinguistTools)
if(WITH_MULTIMEDIA)
  set(_qtComponents ${_qtComponents} Multimedia)
endif()
if(BUILD_TESTING)
  set(_qtComponents ${_qtComponents} Test)
endif()
if(WITH_DBUS)
  set(_qtComponents ${_qtComponents} DBus)
endif()
if(WITH_QML)
  set(_qtComponents ${_qtComponents} Qml Quick)
endif()
if(BUILD_QML_APP)
  set(_qtComponents ${_qtComponents} Svg QuickControls2)
endif()
if(ANDROID AND QT_VERSION_MAJOR LESS 6)
  set(_qtComponents ${_qtComponents} AndroidExtras)
endif()
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${_qtComponents} REQUIRED)

if(WITH_DBUS)
  set(HAVE_QTDBUS 1)
endif()
if(WITH_MULTIMEDIA)
  set(HAVE_QTMULTIMEDIA 1)
endif()
get_target_property(QT_LIBRARY_DIR Qt${QT_VERSION_MAJOR}::Core LOCATION)
get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)
if(APPLE)
  # For Mac Qt5.5.0 QT_LIBRARY_DIR is wrong
  # (<installdir>/Qt5.5.0/5.5/clang_64/lib/QtCore.framework),
  # also for Mac Qt6.2.2 it is wrong
  # (<installdir>/6.2.2/macos/lib/QtCore.framework/Versions/A), fix it.
  string(REGEX MATCH "^(.*)/QtCore.framework.*$" _qtLibDir "${QT_LIBRARY_DIR}")
  if(_qtLibDir)
    set(QT_LIBRARY_DIR ${CMAKE_MATCH_1})
  endif()
  # Fix to find dependencies when Homebrew Qt is installed in its own folder.
  # You may have to "eval $(brew shellenv)" for HOMEBREW_PREFX to be set.
  if(DEFINED ENV{HOMEBREW_PREFIX} AND QT_LIBRARY_DIR STREQUAL "$ENV{HOMEBREW_PREFIX}/opt/qt/lib")
    set(QT_LIBRARY_DIR "$ENV{HOMEBREW_PREFIX}/lib")
  endif()
endif()
if(TARGET Qt${QT_VERSION_MAJOR}::lupdate)
  get_target_property(QT_LUPDATE_EXECUTABLE Qt${QT_VERSION_MAJOR}::lupdate LOCATION)
endif()
if(TARGET Qt${QT_VERSION_MAJOR}::QJpegPlugin)
  get_target_property(QT_PLUGINS_DIR Qt${QT_VERSION_MAJOR}::QJpegPlugin LOCATION)
endif()
if(QT_PLUGINS_DIR)
  get_filename_component(QT_PLUGINS_DIR ${QT_PLUGINS_DIR} PATH)
  get_filename_component(QT_PLUGINS_DIR ${QT_PLUGINS_DIR} PATH)
elseif(_qtDir)
  set(QT_PLUGINS_DIR ${_qtDir}/plugins)
else()
  # Before Qt 5.2 and with Qt 6, there seems to be no way to get the plugins directory.
  # Assume it is on the same level as the bin directory.
  get_target_property(_qmakeLocation Qt${QT_VERSION_MAJOR}::qmake LOCATION)
  get_filename_component(QT_PLUGINS_DIR ${_qmakeLocation} PATH)
  get_filename_component(QT_PLUGINS_DIR ${QT_PLUGINS_DIR} PATH)
  set(QT_PLUGINS_DIR ${QT_PLUGINS_DIR}/plugins)
endif()

if(QT_VERSION VERSION_LESS 5.15)
  function(qt_wrap_cpp outfiles)
    qt5_wrap_cpp("${outfiles}" ${ARGN})
    set("${outfiles}" "${${outfiles}}" PARENT_SCOPE)
  endfunction()

  function(qt_add_resources outfiles)
    qt5_add_resources("${outfiles}" ${ARGN})
    set("${outfiles}" "${${outfiles}}" PARENT_SCOPE)
  endfunction()

  function(qt_add_translation _qm_files)
    qt5_add_translation("${_qm_files}" ${ARGN})
    set("${_qm_files}" "${${_qm_files}}" PARENT_SCOPE)
  endfunction()
endif()

get_target_property(_qmakeLocation Qt${QT_VERSION_MAJOR}::qmake LOCATION)
message(STATUS "Found Qt-Version ${QT_VERSION} (using ${_qmakeLocation})")

# QT_MAJOR_VERSION is used for KDE, to use the same code for KF5 and KF6.
if(NOT QT_MAJOR_VERSION)
  if(QT_VERSION_MAJOR)
    set(QT_MAJOR_VERSION ${QT_VERSION_MAJOR})
  else()
    set(QT_MAJOR_VERSION "5")
  endif()
endif()

if(LINUX_SELF_CONTAINED)
  set(CPACK_INSTALL_PREFIX "")
  # Avoid GLIBC_2.14 not found errors on older systems like Debian Wheezy.
  # https://stackoverflow.com/questions/8823267/linking-against-older-symbol-version-in-a-so-file
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/linux/glibc_version_nightmare.h")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/linux/glibc_version_nightmare.h")
  # Use RPATH instead of RUNPATH, the RPATH of the loading executable has to be used, see
  # https://www.qt.io/blog/2011/10/28/rpath-and-runpath
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--disable-new-dtags")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")
endif()

# Check that QT_NO_DEBUG is defined for release configurations
foreach(_buildType RELWITHDEBINFO RELEASE MINSIZEREL)
  if(NOT CMAKE_CXX_FLAGS_${_buildType} MATCHES "-DQT_NO_DEBUG")
    set(CMAKE_CXX_FLAGS_${_buildType} "${CMAKE_CXX_FLAGS_${_buildType}} -DQT_NO_DEBUG")
  endif()
endforeach()

### Check for xsltproc
find_program(XSLTPROC xsltproc DOC "xsltproc transforms XML via XSLT"
  PATHS $ENV{XSLTPROCDIR})
if(NOT XSLTPROC)
  message(FATAL_ERROR "Could not find xsltproc")
endif()

### Check for HTML docbook.xsl
file(GLOB _versionedStyleSheetDir /usr/share/xml/docbook/xsl-stylesheets-*)
set(_docbookdirPaths /usr/share/xml/docbook/stylesheet/nwalsh
    /usr/share/xml/docbook/stylesheet/nwalsh/current
    /usr/share/xml/docbook/stylesheet/docbook-xsl
    /usr/share/xml/docbook/stylesheet/docbook-xsl-ns
    /usr/share/sgml/docbook/xsl-stylesheets
    /usr/share/apps/ksgmltools2/docbook/xsl)
if(WITH_DOCBOOKDIR)
  list(PREPEND _docbookdirPaths ${WITH_DOCBOOKDIR})
endif()
if(_versionedStyleSheetDir)
  list(APPEND _docbookdirPaths ${_versionedStyleSheetDir})
endif()
if(DEFINED ENV{DOCBOOKDIR})
  list(APPEND _docbookdirPaths $ENV{DOCBOOKDIR})
endif()
if(DEFINED ENV{HOME})
  list(APPEND _docbookdirPaths $ENV{HOME}/docbook-xsl-1.72.0)
endif()
find_path(DOCBOOK_XSL_DIR xhtml/docbook.xsl
  PATHS ${_docbookdirPaths} NO_DEFAULT_PATH)
if(NOT DOCBOOK_XSL_DIR)
  message(FATAL_ERROR "Could not find HTML docbook.xsl")
endif()

if(WIN32)
  link_directories($ENV{LIB})
endif()

if(BUILD_SHARED_LIBS)
  add_definitions(-DKID3_SHARED)

  # the RPATH to be used when installing, but only if it's not a system directory
  # see https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
  set(_prefixLibdir "${CMAKE_INSTALL_PREFIX}/${WITH_LIBDIR}")
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${_prefixLibdir}" _isSystemDir)
  if("${_isSystemDir}" STREQUAL "-1")
    list(FIND CMAKE_INSTALL_RPATH "${_prefixLibdir}" _rpathContainsLibdir)
    if("${_rpathContainsLibdir}" STREQUAL "-1")
      list(APPEND CMAKE_INSTALL_RPATH "${_prefixLibdir}")
      if(CMAKE_SKIP_RPATH AND NOT ANDROID)
        message(WARNING
          "RPATH needs to be set to \"${_prefixLibdir}\" but CMAKE_SKIP_RPATH "
          "is set! Disable CMAKE_SKIP_RPATH or set WITH_LIBDIR (which is now "
          "\"${WITH_LIBDIR}\") to a system directory, "
          "e.g. \"-DWITH_LIBDIR=lib\".")
      endif()
    endif()
  endif()
endif()

### Check for zlib
if(WIN32)
  if(NOT MSVC)
    find_library(_ZLIB_PATH_NAME z)
    if(_ZLIB_PATH_NAME)
      get_filename_component(_ZLIB_PATH ${_ZLIB_PATH_NAME} PATH)
    else()
      message(FATAL_ERROR "Could not find zlib")
    endif()
    set(ZLIB_LIBRARIES "-L${_ZLIB_PATH}" -lz)
  endif()
else()
  set(ZLIB_LIBRARIES -lz)
endif()

### Check for QML
if(WITH_QML AND (QT_QTDECLARATIVE_FOUND OR Qt${QT_VERSION_MAJOR}Qml_FOUND))
  set(HAVE_QML 1)
endif()

### Check for libQtDBus
if(WITH_DBUS AND QT_QTDBUS_LIBRARY)
  set(HAVE_QTDBUS 1)
endif()

### Check for mntent.h
include(CheckIncludeFile)
check_include_file("mntent.h" HAVE_MNTENT_H)


if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -D_REENTRANT")
endif()
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -fno-check-new -fno-common -D_REENTRANT")
  check_cxx_compiler_flag(-Woverloaded-virtual _HAVE_W_OVERLOADED_VIRTUAL)
  if(_HAVE_W_OVERLOADED_VIRTUAL)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
  endif()
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -D_REENTRANT")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -Woverloaded-virtual -fno-common -Werror=return-type -Wweak-vtables -D_REENTRANT")
endif()
if(APPLE)
  execute_process(COMMAND sw_vers -productVersion OUTPUT_VARIABLE _osxVersion
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(_osxVersion VERSION_LESS "10.10" AND NOT CMAKE_CXX_COMPILER MATCHES "/osxcross/")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.5")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5")
  else()
    if(QT_VERSION_MAJOR EQUAL 6 AND QT_VERSION_MINOR GREATER_EQUAL 5)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.15")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.15 -stdlib=libc++")
    else()
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.7")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.7 -stdlib=libc++")
    endif()
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -stdlib=libc++")
  endif()
endif()
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

if(NOT WIN32)
  check_cxx_compiler_flag(-fvisibility=hidden _HAVE_GCC_VISIBILITY)
  if(_HAVE_GCC_VISIBILITY)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  endif()
endif()

if(UNIX AND NOT APPLE)
  set(KID3_EXECUTABLE kid3-qt)
else()
  set(KID3_EXECUTABLE kid3)
endif()

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if(APPLE AND CMAKE_STRIP)
  # Do something against these error messages starting with
  # strip: symbols referenced by indirect symbol table entries that can't be stripped in
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/strip_silent.sh.in
                 ${CMAKE_CURRENT_BINARY_DIR}/strip_silent.sh
                 @ONLY)
  set(CMAKE_STRIP "${CMAKE_CURRENT_BINARY_DIR}/strip_silent.sh")
endif()

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(translations)

if(ANDROID)
  add_subdirectory(android)
endif()


if(BUILD_TESTING)
  add_test(NAME test_cli_functions
           COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/src/test/test_cli_functions.py)
  add_custom_target(check ${CMAKE_CTEST_COMMAND} --output-on-failure)
  if(NOT CMAKE_VERSION VERSION_LESS "3.17")
    list(APPEND CMAKE_CTEST_ARGUMENTS --output-on-failure)
  endif()
endif()


# To create a package, run cpack
if(APPLE)
  if(CMAKE_HOST_APPLE)
    install(SCRIPT ${CMAKE_SOURCE_DIR}/macosx/mac-codesign.cmake)
    # KDE's appleutils.getDmgApplicationId() searches for the Info.plist file
    # containing the application ID in CFBundleIdentifier using a "*.dmg/*.app"
    # wildcard, so the volume name has to be adapted.
    set(CPACK_DMG_VOLUME_NAME "kid3.dmg")
  endif()
  set(CPACK_BINARY_DRAGNDROP ON)
  set(CPACK_GENERATOR DragNDrop)
elseif(WIN32)
  set(CPACK_GENERATOR ZIP)
elseif(LINUX_SELF_CONTAINED)
  set(CPACK_GENERATOR TGZ)
else()
  set(CPACK_GENERATOR DEB)
endif()

if(BUILD_KDE_APP OR APPLE OR WIN32 OR LINUX_SELF_CONTAINED)
  set(CPACK_PACKAGE_NAME "kid3")
  set(CPACK_DEBIAN_PACKAGE_SECTION "kde")
else()
  set(CPACK_PACKAGE_NAME "kid3-qt")
  set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
endif()
set(CPACK_PACKAGE_CONTACT "ufleisch@users.sourceforge.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Audio tag editor")
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://kid3.kde.org/")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_SET_DESTDIR ON)

include(CPack)
