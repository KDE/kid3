# Build Kid3 using CMake
#
# Linux:
# cmake -D CMAKE_BUILD_TYPE=Release -D WITH_APPS=Qt -D CMAKE_INSTALL_PREFIX=/usr ../kid3
# make
# make install/strip DESTDIR=$(pwd)/inst # or
# cpack
#
# Windows:
# set INCLUDE=%MSYSDIR%\local\include
# set LIB=%MSYSDIR%\local\lib
# cmake -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= ..\kid3
# mingw32-make
# cpack
#
# Mac OS X:
# cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= ../kid3
# make
# cpack

project(kid3)

cmake_minimum_required(VERSION 3.3)

set(BUILD_SHARED_LIBS ON CACHE BOOL "build shared libraries")
set(WITH_QML    ON CACHE BOOL "build with QML")
set(WITH_TAGLIB ON CACHE BOOL "build with TagLib")
set(WITH_MP4V2 OFF CACHE BOOL "build with mp4v2")
set(WITH_FFMPEG OFF CACHE BOOL "force use of FFmpeg for Chromaprint decoding")
set(WITH_CHROMAPRINT_FFMPEG OFF CACHE BOOL "link FFmpeg only for Chromaprint FFT")
set(WITH_GSTREAMER OFF CACHE BOOL "force use of GStreamer for Chromaprint decoding")
set(WITH_QAUDIODECODER OFF CACHE BOOL "force use of QAudioDecoder for Chromaprint decoding")
set(WITH_GCC_PCH OFF CACHE BOOL "enable precompiled headers")
set(WITH_NO_MANCOMPRESS OFF CACHE BOOL "disable compressed manpages")
if (ANDROID)
  set(CMAKE_INSTALL_PREFIX "" CACHE PATH "install path prefix" FORCE)
  set(WITH_ID3LIB OFF CACHE BOOL "build with id3lib")
  set(WITH_VORBIS OFF CACHE BOOL "build with Ogg/Vorbis")
  set(WITH_FLAC OFF CACHE BOOL "build with FLAC")
  set(WITH_CHROMAPRINT OFF CACHE BOOL "build with Chromaprint")
else (ANDROID)
  set(WITH_ID3LIB ON CACHE BOOL "build with id3lib")
  set(WITH_VORBIS ON CACHE BOOL "build with Ogg/Vorbis")
  set(WITH_FLAC ON CACHE BOOL "build with FLAC")
  set(WITH_CHROMAPRINT ON CACHE BOOL "build with Chromaprint")
endif (ANDROID)
if (APPLE OR WIN32)
  set(WITH_DBUS OFF CACHE BOOL "build with QtDBus")
  set(WITH_APPS "Qt;CLI" CACHE STRING "build applications (Qt;CLI;Test)")
elseif (ANDROID)
  set(WITH_DBUS OFF CACHE BOOL "build with QtDBus")
  set(WITH_APPS "Qml" CACHE STRING "build applications (Qml;Qt;CLI;Test)")
else (APPLE OR WIN32)
  set(WITH_DBUS ON CACHE BOOL "build with QtDBus")
  set(WITH_APPS "Qt;CLI;KDE" CACHE STRING "build applications (Qt;CLI;KDE;Test)")
endif (APPLE OR WIN32)
if (WIN32)
  set(WITH_READLINE OFF CACHE BOOL "build with readline")
else (WIN32)
  set(WITH_READLINE ON CACHE BOOL "build with readline")
endif (WIN32)

if (DEFINED WITH_KDE)
  unset(WITH_KDE CACHE)
  message(FATAL_ERROR "The variable WITH_KDE is no longer used. The KDE and Qt "
                      "applications can now be built together using common "
                      "libraries. Use WITH_APPS with a value of \"KDE\" for "
                      "the KDE application, \"Qt\" for the Qt application, "
                      "\"CLI\" for the command line application or "
                      "\"Qt;CLI;KDE\" for all. Default is -DWITH_APPS="
                      "\"${WITH_APPS}\".")
endif (DEFINED WITH_KDE)

if (WITH_APPS)
  string(TOLOWER "${WITH_APPS}" _apps_lower)
  foreach (_app ${_apps_lower})
    if (_app STREQUAL "qt")
      set(BUILD_QT_APP ON)
    elseif (_app STREQUAL "qml")
      set(BUILD_QML_APP ON)
      set(WITH_QML ON)
    elseif (_app STREQUAL "cli")
      set(BUILD_CLI_APP ON)
    elseif (_app STREQUAL "kde")
      set(BUILD_KDE_APP ON)
    elseif (_app STREQUAL "test")
      set(BUILD_TEST_APP ON)
    else ()
      message(SEND_ERROR "Invalid value ${_app} in WITH_APPS. "
                         "Supported applications are \"Qt;KDE;Test\".")
    endif ()
  endforeach ()
endif (WITH_APPS)

if (NOT BUILD_SHARED_LIBS AND WITH_QML)
  message(WARNING "Switching off WITH_QML because of static build")
  set(WITH_QML OFF)
  set(BUILD_QML_APP OFF)
endif (NOT BUILD_SHARED_LIBS AND WITH_QML)


# Version information
set(CPACK_PACKAGE_VERSION_MAJOR 3)
set(CPACK_PACKAGE_VERSION_MINOR 6)
set(CPACK_PACKAGE_VERSION_PATCH 2)
set(KID3_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION ${KID3_VERSION})
#set(CPACK_PACKAGE_VERSION "git20170517")
set(RELEASE_YEAR 2018)

# Platform specific default installation paths
if (APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_DOCDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_TRANSLATIONSDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_QMLDIR_DEFAULT "kid3.app/Contents/Resources/qml")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT "kid3.app/Contents/MacOS")
  set(WITH_PLUGINSDIR_DEFAULT "kid3.app/Contents/PlugIns")
elseif (WIN32)
  set(WITH_DATAROOTDIR_DEFAULT ".")
  set(WITH_DOCDIR_DEFAULT ".")
  set(WITH_TRANSLATIONSDIR_DEFAULT ".")
  set(WITH_QMLDIR_DEFAULT "./qml")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT ".")
  set(WITH_PLUGINSDIR_DEFAULT "./plugins")
elseif (ANDROID)
  set(WITH_DATAROOTDIR_DEFAULT ".")
  set(WITH_DOCDIR_DEFAULT ".")
  set(WITH_TRANSLATIONSDIR_DEFAULT ":/translations")
  set(WITH_QMLDIR_DEFAULT ":/")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT ".")
  set(WITH_PLUGINSDIR_DEFAULT ".")
  set(QT_ANDROID_APP_VERSION ${KID3_VERSION})
  set(QT_ANDROID_APP_VERSION_CODE 13)
else (APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "share")
  set(WITH_DOCDIR_DEFAULT "share/doc/kid3-qt")
  if (BUILD_KDE_APP OR BUILD_SHARED_LIBS)
    set(WITH_TRANSLATIONSDIR_DEFAULT "share/kid3/translations")
  else (BUILD_KDE_APP OR BUILD_SHARED_LIBS)
    set(WITH_TRANSLATIONSDIR_DEFAULT "share/kid3-qt/translations")
  endif (BUILD_KDE_APP OR BUILD_SHARED_LIBS)
  set(WITH_QMLDIR_DEFAULT "share/kid3/qml")
  set(WITH_BINDIR_DEFAULT "bin")
  set(WITH_LIBDIR_DEFAULT "lib${LIB_SUFFIX}/kid3")
  set(WITH_PLUGINSDIR_DEFAULT "${WITH_LIBDIR_DEFAULT}/plugins")
endif (APPLE)

set(WITH_DATAROOTDIR ${WITH_DATAROOTDIR_DEFAULT} CACHE STRING
    "data root directory relative to CMAKE_INSTALL_PREFIX")
if (BUILD_KDE_APP)
  set(WITH_DBUSDIR "share/dbus-1/interfaces" CACHE STRING
      "dbus directory relative to CMAKE_INSTALL_PREFIX")
endif (BUILD_KDE_APP)
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
if (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "${CMAKE_INSTALL_PREFIX}/")
else (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "")
endif (CMAKE_INSTALL_PREFIX)

if (APPLE)
file(RELATIVE_PATH CFG_DATAROOTDIR "/kid3.app/Contents/MacOS" /${WITH_DATAROOTDIR})
file(RELATIVE_PATH CFG_DOCDIR "/kid3.app/Contents/MacOS" /${WITH_DOCDIR})
file(RELATIVE_PATH CFG_TRANSLATIONSDIR "/kid3.app/Contents/MacOS" /${WITH_TRANSLATIONSDIR})
file(RELATIVE_PATH CFG_QMLDIR "/kid3.app/Contents/MacOS" /${WITH_QMLDIR})
file(RELATIVE_PATH CFG_PLUGINSDIR "/kid3.app/Contents/MacOS" /${WITH_PLUGINSDIR})
else (APPLE)
set(CFG_DATAROOTDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DATAROOTDIR})
set(CFG_DOCDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DOCDIR})
set(CFG_TRANSLATIONSDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_TRANSLATIONSDIR})
set(CFG_QMLDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_QMLDIR})
file(RELATIVE_PATH CFG_PLUGINSDIR /${WITH_BINDIR} /${WITH_PLUGINSDIR})
endif (APPLE)
if (CFG_PLUGINSDIR STREQUAL "")
  set(CFG_PLUGINSDIR ".")
endif (CFG_PLUGINSDIR STREQUAL "")
if (WITH_QMLDIR MATCHES "^:/")
  set(HAVE_QMLDIR_IN_QRC 1)
endif (WITH_QMLDIR MATCHES "^:/")
if (WITH_TRANSLATIONSDIR MATCHES "^:/")
  set(HAVE_TRANSLATIONSDIR_IN_QRC 1)
endif (WITH_TRANSLATIONSDIR MATCHES "^:/")
set(CFG_QMLSRCDIR "${kid3_SOURCE_DIR}/src/qml")

find_package(Threads)
find_package(PythonInterp REQUIRED)
include (CheckCXXSourceCompiles)
include (CheckCXXCompilerFlag)
include (CheckLibraryExists)

# Find Qt
set(_qt5Dir)
string(REGEX MATCH "^(.*[Qq]t[/\\]?5.*).bin.qmake.*" _qt5Dir "${QT_QMAKE_EXECUTABLE}")
if (_qt5Dir)
  set(_qt5Dir ${CMAKE_MATCH_1})
endif (_qt5Dir)

if (_qt5Dir)
  set(Qt5Core_DIR "${_qt5Dir}/lib/cmake/Qt5Core")
  find_package(Qt5Core)
  if (Qt5Core_FOUND)
    message(STATUS "Qt5Core found in ${_qt5Dir}")
    set(Qt5_DIR "${_qt5Dir}/lib/cmake/Qt5")
    set(Qt5Gui_DIR "${_qt5Dir}/lib/cmake/Qt5Gui")
    set(Qt5Widgets_DIR "${_qt5Dir}/lib/cmake/Qt5Widgets")
    set(Qt5Network_DIR "${_qt5Dir}/lib/cmake/Qt5Network")
    set(Qt5Xml_DIR "${_qt5Dir}/lib/cmake/Qt5Xml")
    set(Qt5Multimedia_DIR "${_qt5Dir}/lib/cmake/Qt5Multimedia")
    set(Qt5LinguistTools_DIR "${_qt5Dir}/lib/cmake/Qt5LinguistTools")
    set(Qt5Test_DIR "${_qt5Dir}/lib/cmake/Qt5Test")
    if (WITH_DBUS)
      set(Qt5DBus_DIR "${_qt5Dir}/lib/cmake/Qt5DBus")
    endif (WITH_DBUS)
    if (WITH_QML)
      set(Qt5Qml_DIR "${_qt5Dir}/lib/cmake/Qt5Qml")
      set(Qt5Quick_DIR "${_qt5Dir}/lib/cmake/Qt5Quick")
    endif (WITH_QML)
    if (BUILD_QML_APP)
      set(Qt5Svg_DIR "${_qt5Dir}/lib/cmake/Qt5Svg")
      set(Qt5QuickControls2_DIR "${_qt5Dir}/lib/cmake/Qt5QuickControls2")
    endif (BUILD_QML_APP)
  endif (Qt5Core_FOUND)
else (_qt5Dir)
  find_package(Qt5Core)
endif (_qt5Dir)
if (Qt5Core_FOUND)
  set(_qtComponents Core Gui Widgets Network Xml Multimedia LinguistTools Test)
  if (WITH_DBUS)
    set(_qtComponents ${_qtComponents} DBus)
  endif (WITH_DBUS)
  if (WITH_QML)
    set(_qtComponents ${_qtComponents} Qml Quick)
  endif (WITH_QML)
  if (BUILD_QML_APP)
    set(_qtComponents ${_qtComponents} Svg QuickControls2)
  endif (BUILD_QML_APP)
  find_package(Qt5 COMPONENTS ${_qtComponents} REQUIRED)

  if (WITH_DBUS)
    set(HAVE_QTDBUS 1)
  endif ()
  get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
  get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)
  if (APPLE)
    # For Mac Qt5.5.0 QT_LIBRARY_DIR is wrong
    # (<installdir>/Qt5.5.0/5.5/clang_64/lib/QtCore.framework), fix it.
    string(REGEX MATCH "^(.*)/QtCore.framework$" _qt5LibDir "${QT_LIBRARY_DIR}")
    if (_qt5LibDir)
      set(QT_LIBRARY_DIR ${CMAKE_MATCH_1})
    endif (_qt5LibDir)
  endif (APPLE)
  if (TARGET Qt5::lupdate)
    get_target_property(QT_LUPDATE_EXECUTABLE Qt5::lupdate LOCATION)
  endif ()
  if (TARGET Qt5::QJpegPlugin)
    get_target_property(QT_PLUGINS_DIR Qt5::QJpegPlugin LOCATION)
  endif ()
  if (QT_PLUGINS_DIR)
    get_filename_component(QT_PLUGINS_DIR ${QT_PLUGINS_DIR} PATH)
    get_filename_component(QT_PLUGINS_DIR ${QT_PLUGINS_DIR} PATH)
  elseif (_qt5Dir)
    set(QT_PLUGINS_DIR ${_qt5Dir}/plugins)
  else (QT_PLUGINS_DIR)
    # Before Qt 5.2, there seems to be no way to get the plugins directory.
    # Assume it is on the same level as the bin directory.
    get_filename_component(QT_PLUGINS_DIR ${QT_LUPDATE_EXECUTABLE} PATH)
    get_filename_component(QT_PLUGINS_DIR ${QT_PLUGINS_DIR} PATH)
    set(QT_PLUGINS_DIR ${QT_PLUGINS_DIR}/plugins)
  endif (QT_PLUGINS_DIR)

  get_target_property(_qmakeLocation Qt5::qmake LOCATION)
  message(STATUS "Found Qt-Version ${Qt5Core_VERSION_STRING} (using ${_qmakeLocation})")
endif (Qt5Core_FOUND)

if (NOT Qt5Core_FOUND)
  message(FATAL_ERROR "Qt5 not found.")
endif ()

if (LINUX_SELF_CONTAINED)
  set(CPACK_INSTALL_PREFIX "")
  # Avoid GLIBC_2.14 not found errors on older systems like Debian Wheezy.
  # http://stackoverflow.com/questions/8823267/linking-against-older-symbol-version-in-a-so-file
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/linux/glibc_version_nightmare.h")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/linux/glibc_version_nightmare.h")
  # Use RPATH instead of RUNPATH, the RPATH of the loading executable has to be used, see
  # http://blog.qt.io/blog/2011/10/28/rpath-and-runpath/
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--disable-new-dtags")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")
endif ()

# Check that QT_NO_DEBUG is defined for release configurations
foreach (_buildType RELWITHDEBINFO RELEASE MINSIZEREL)
  if (NOT CMAKE_CXX_FLAGS_${_buildType} MATCHES "-DQT_NO_DEBUG")
    set(CMAKE_CXX_FLAGS_${_buildType} "${CMAKE_CXX_FLAGS_${_buildType}} -DQT_NO_DEBUG")
  endif ()
endforeach ()

### Check for xsltproc
find_program(XSLTPROC xsltproc DOC "xsltproc transforms XML via XSLT"
  PATHS $ENV{XSLTPROCDIR})
if (NOT XSLTPROC)
  message(FATAL_ERROR "Could not find xsltproc")
endif (NOT XSLTPROC)

### Check for HTML docbook.xsl
file(GLOB _versionedStyleSheetDir /usr/share/xml/docbook/xsl-stylesheets-*)
find_path(DOCBOOK_XSL_DIR html/docbook.xsl
  PATHS ${WITH_DOCBOOKDIR}
        /usr/share/xml/docbook/stylesheet/nwalsh
        /usr/share/xml/docbook/stylesheet/nwalsh/current
        /usr/share/xml/docbook/stylesheet/docbook-xsl
        /usr/share/xml/docbook/stylesheet/docbook-xsl-ns
        /usr/share/sgml/docbook/xsl-stylesheets
        /usr/share/apps/ksgmltools2/docbook/xsl
       ${_versionedStyleSheetDir}
       $ENV{DOCBOOKDIR}
       $ENV{HOME}/docbook-xsl-1.72.0
  NO_DEFAULT_PATH)
if (NOT DOCBOOK_XSL_DIR)
  message(FATAL_ERROR "Could not find HTML docbook.xsl")
endif (NOT DOCBOOK_XSL_DIR)

if (WIN32)
  link_directories($ENV{LIB})
endif (WIN32)

if (BUILD_SHARED_LIBS)
  add_definitions(-DKID3_SHARED)

  # the RPATH to be used when installing, but only if it's not a system directory
  # see http://www.cmake.org/Wiki/CMake_RPATH_handling
  set(_prefixLibdir "${CMAKE_INSTALL_PREFIX}/${WITH_LIBDIR}")
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${_prefixLibdir}" _isSystemDir)
  if ("${_isSystemDir}" STREQUAL "-1")
    list(FIND CMAKE_INSTALL_RPATH "${_prefixLibdir}" _rpathContainsLibdir)
    if ("${_rpathContainsLibdir}" STREQUAL "-1")
      list(APPEND CMAKE_INSTALL_RPATH "${_prefixLibdir}")
      if (CMAKE_SKIP_RPATH AND NOT ANDROID)
        message(WARNING
          "RPATH needs to be set to \"${_prefixLibdir}\" but CMAKE_SKIP_RPATH "
          "is set! Disable CMAKE_SKIP_RPATH or set WITH_LIBDIR (which is now "
          "\"${WITH_LIBDIR}\") to a system directory, "
          "e.g. \"-DWITH_LIBDIR=lib\".")
      endif (CMAKE_SKIP_RPATH AND NOT ANDROID)
    endif ("${_rpathContainsLibdir}" STREQUAL "-1")
  endif ("${_isSystemDir}" STREQUAL "-1")
endif (BUILD_SHARED_LIBS)

### Check for zlib
if (WIN32)
  if (NOT MSVC)
    find_library(_ZLIB_PATH_NAME z)
    if (_ZLIB_PATH_NAME)
      get_filename_component(_ZLIB_PATH ${_ZLIB_PATH_NAME} PATH)
    else (_ZLIB_PATH_NAME)
      message(FATAL_ERROR "Could not find zlib")
    endif (_ZLIB_PATH_NAME)
    set(ZLIB_LIBRARIES "-L${_ZLIB_PATH}" -lz)
  endif (NOT MSVC)
else (WIN32)
  set(ZLIB_LIBRARIES -lz)
endif (WIN32)

### Check for QML
if (WITH_QML AND (QT_QTDECLARATIVE_FOUND OR Qt5Qml_FOUND))
  set(HAVE_QML 1)
endif (WITH_QML AND (QT_QTDECLARATIVE_FOUND OR Qt5Qml_FOUND))

### Check for libQtDBus
if (WITH_DBUS AND QT_QTDBUS_LIBRARY)
  set(HAVE_QTDBUS 1)
endif (WITH_DBUS AND QT_QTDBUS_LIBRARY)

### Check for mntent.h
include(CheckIncludeFile)
check_include_file("mntent.h" HAVE_MNTENT_H)

set(QT_DEFINITIONS ${QT_DEFINITIONS}
  -DQT_ASCII_CAST_WARNINGS -DQT_NO_CAST_TO_ASCII -DQT_NO_URL_CAST_FROM_STRING -DQT_STRICT_ITERATORS)


if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -D_REENTRANT")
endif (CMAKE_COMPILER_IS_GNUCC)
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -fno-check-new -fno-common -D_REENTRANT")
  check_cxx_compiler_flag(-Woverloaded-virtual _HAVE_W_OVERLOADED_VIRTUAL)
  if(_HAVE_W_OVERLOADED_VIRTUAL)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
  endif(_HAVE_W_OVERLOADED_VIRTUAL)
endif (CMAKE_COMPILER_IS_GNUCXX)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -D_REENTRANT")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -Woverloaded-virtual -fno-common -Werror=return-type -Wweak-vtables -D_REENTRANT")
endif ()
if (MSVC)
  # Treat wchar_t as built-in type, else QString::fromWCharArray is unresolved
  add_definitions("/Zc:wchar_t-")
endif (MSVC)
if (APPLE)
  exec_program(sw_vers ARGS -productVersion OUTPUT_VARIABLE _osxVersion)
  if (_osxVersion VERSION_LESS "10.10" AND NOT CMAKE_CXX_COMPILER MATCHES "/osxcross/")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.5")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5")
  else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.7")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.7 -stdlib=libc++")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -stdlib=libc++")
  endif ()
endif (APPLE)
add_definitions(${QT_DEFINITIONS} -DHAVE_CONFIG_H=1 -DQT_NO_EXCEPTIONS -DQT_NO_STL)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

if (NOT WIN32)
  check_cxx_compiler_flag(-fvisibility=hidden _HAVE_GCC_VISIBILITY)
  if (_HAVE_GCC_VISIBILITY)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  endif (_HAVE_GCC_VISIBILITY)
endif (NOT WIN32)

if (UNIX AND NOT APPLE)
  set(KID3_EXECUTABLE kid3-qt)
else (UNIX AND NOT APPLE)
  set(KID3_EXECUTABLE kid3)
endif (UNIX AND NOT APPLE)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(po)

if (ANDROID)
  add_subdirectory(android)
endif (ANDROID)


# To create a package, run cpack
if (APPLE)
  set(CPACK_BINARY_DRAGNDROP ON)
  set(CPACK_GENERATOR DragNDrop)
elseif (WIN32)
  set(CPACK_GENERATOR ZIP)
elseif (LINUX_SELF_CONTAINED)
  set(CPACK_GENERATOR TGZ)
else (APPLE)
  set(CPACK_GENERATOR DEB)
endif (APPLE)

if (BUILD_KDE_APP OR APPLE OR WIN32 OR LINUX_SELF_CONTAINED)
  set(CPACK_PACKAGE_NAME "kid3")
  set(CPACK_DEBIAN_PACKAGE_SECTION "kde")
else ()
  set(CPACK_PACKAGE_NAME "kid3-qt")
  set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
endif ()
set(CPACK_PACKAGE_CONTACT "ufleisch@users.sourceforge.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Audio tag editor")
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://kid3.sourceforge.net/")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
