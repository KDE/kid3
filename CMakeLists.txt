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
# cmake -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= -D WITH_CHROMAPRINT=OFF ..\kid3
# mingw32-make
# cpack
#
# Mac OS X:
# cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= -D WITH_CHROMAPRINT=OFF ../kid3
# make
# cpack

project(kid3)

cmake_minimum_required(VERSION 2.8)

set(WITH_TAGLIB ON CACHE BOOL "build with TagLib")
set(WITH_MP4V2 OFF CACHE BOOL "build with mp4v2")
set(WITH_ID3LIB ON CACHE BOOL "build with id3lib")
set(WITH_VORBIS ON CACHE BOOL "build with Ogg/Vorbis")
set(WITH_FLAC ON CACHE BOOL "build with FLAC")
set(WITH_CHROMAPRINT ON CACHE BOOL "build with Chromaprint")
set(WITH_FFMPEG OFF CACHE BOOL "force use of FFmpeg for Chromaprint decoding")
set(WITH_CHROMAPRINT_FFMPEG OFF CACHE BOOL "link FFmpeg only for Chromaprint FFT")
set(WITH_GSTREAMER OFF CACHE BOOL "force use of GStreamer for Chromaprint decoding")
set(WITH_QAUDIODECODER OFF CACHE BOOL "force use of QAudioDecoder for Chromaprint decoding")
set(WITH_DBUS ON CACHE BOOL "build with QtDBus")
set(WITH_PHONON ON CACHE BOOL "build with Phonon")
set(WITH_GCC_PCH OFF CACHE BOOL "enable precompiled headers")
if (APPLE OR WIN32)
  set(WITH_APPS "Qt" CACHE STRING "build applications (Qt;Test)")
else (APPLE OR WIN32)
  set(WITH_APPS "Qt;KDE;Test" CACHE STRING "build applications (Qt;KDE;Test)")
endif (APPLE OR WIN32)

if (DEFINED WITH_KDE)
  unset(WITH_KDE CACHE)
  message(FATAL_ERROR "The variable WITH_KDE is no longer used. The KDE and Qt "
                      "applications can now be built together using common "
                      "libraries. Use WITH_APPS with a value of \"KDE\" for "
                      "the KDE application, \"Qt\" for the Qt application or "
                      "\"Qt;KDE\" for both. Default is -DWITH_APPS="
                      "\"${WITH_APPS}\". If you build multiple applications, "
                      "consider using -DBUILD_SHARED_LIBS=ON to build shared "
                      "dynamic libraries.")
endif (DEFINED WITH_KDE)

if (WITH_APPS)
  string(TOLOWER "${WITH_APPS}" _apps_lower)
  foreach (_app ${_apps_lower})
    if (_app STREQUAL "qt")
      set(BUILD_QT_APP ON)
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

# Version information
set(CPACK_PACKAGE_VERSION_MAJOR 2)
set(CPACK_PACKAGE_VERSION_MINOR 3)
set(CPACK_PACKAGE_VERSION_PATCH 0)
#set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "git20130804")
set(RELEASE_YEAR 2013)

# Platform specific default installation paths
if (APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_DOCDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_TRANSLATIONSDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT "kid3.app/Contents/MacOS")
  set(WITH_PLUGINSDIR_DEFAULT "kid3.app/Contents/PlugIns")
elseif (WIN32)
  set(WITH_DATAROOTDIR_DEFAULT ".")
  set(WITH_DOCDIR_DEFAULT ".")
  set(WITH_TRANSLATIONSDIR_DEFAULT ".")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT ".")
  set(WITH_PLUGINSDIR_DEFAULT "./plugins")
else (APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "share")
  set(WITH_DOCDIR_DEFAULT "share/doc/kid3-qt")
  if (BUILD_KDE_APP OR BUILD_SHARED_LIBS)
    set(WITH_TRANSLATIONSDIR_DEFAULT "share/kid3/translations")
  else (BUILD_KDE_APP OR BUILD_SHARED_LIBS)
    set(WITH_TRANSLATIONSDIR_DEFAULT "share/kid3-qt/translations")
  endif (BUILD_KDE_APP OR BUILD_SHARED_LIBS)
  set(WITH_BINDIR_DEFAULT "bin")
  set(WITH_LIBDIR_DEFAULT "lib/kid3")
  set(WITH_PLUGINSDIR_DEFAULT "lib/kid3/plugins")
endif (APPLE)

set(WITH_DATAROOTDIR ${WITH_DATAROOTDIR_DEFAULT} CACHE STRING
    "data root directory relative to CMAKE_INSTALL_PREFIX")
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
if (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "${CMAKE_INSTALL_PREFIX}/")
else (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "")
endif (CMAKE_INSTALL_PREFIX)

set(CFG_DATAROOTDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DATAROOTDIR})
set(CFG_DOCDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DOCDIR})
set(CFG_TRANSLATIONSDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_TRANSLATIONSDIR})
if (APPLE)
file(RELATIVE_PATH CFG_PLUGINSDIR "/kid3.app/Contents/MacOS" /${WITH_PLUGINSDIR})
else (APPLE)
file(RELATIVE_PATH CFG_PLUGINSDIR /${WITH_BINDIR} /${WITH_PLUGINSDIR})
endif (APPLE)

if (WITH_GCC_PCH)
include(GccPchMacros.cmake)
endif (WITH_GCC_PCH)

find_package(Threads)
find_package(Perl REQUIRED)
include (CheckCXXSourceCompiles)
include (CheckCXXCompilerFlag)
include (CheckLibraryExists)


set(QT_EXECUTABLE_COMPILE_FLAGS)
string(REGEX MATCH "^(.*[Qq]t5.*).bin.qmake.*" _qt5Dir "${QT_QMAKE_EXECUTABLE}")
if (_qt5Dir)
  set(_qt5Dir ${CMAKE_MATCH_1})
endif (_qt5Dir)

if (NOT _qt5Dir)
  set(_QT4_COMPONENTS QtCore QtGui QtNetwork QtXml)
  if (WIN32)
    set(QT_USE_QTMAIN TRUE)
  endif (WIN32)
  set(QT_USE_QTDBUS ${WITH_DBUS})
  find_package(Qt4 COMPONENTS ${_QT4_COMPONENTS})
  if (QT_QMAKE_EXECUTABLE)
    include (${QT_USE_FILE})
    message(STATUS "Qt4 found, explicitly set QT_QMAKE_EXECUTABLE to use a different version.")
  else (QT_QMAKE_EXECUTABLE)
    find_package(Qt5Core)
    if (Qt5Core_QMAKE_EXECUTABLE)
      set(_qt5Dir ${_qt5_corelib_install_prefix})
    else (Qt5Core_QMAKE_EXECUTABLE)
      message(FATAL_ERROR "Neither Qt4 nor Qt5 found.")
    endif (Qt5Core_QMAKE_EXECUTABLE)
  endif (QT_QMAKE_EXECUTABLE)
endif (NOT _qt5Dir)

if (_qt5Dir)
  set(HAVE_QT5 1)
  set(WITH_PHONON OFF)
  set(Qt5Core_DIR "${_qt5Dir}/lib/cmake/Qt5Core")
  set(Qt5Gui_DIR "${_qt5Dir}/lib/cmake/Qt5Gui")
  set(Qt5Widgets_DIR "${_qt5Dir}/lib/cmake/Qt5Widgets")
  set(Qt5Network_DIR "${_qt5Dir}/lib/cmake/Qt5Network")
  set(Qt5Xml_DIR "${_qt5Dir}/lib/cmake/Qt5Xml")
  set(Qt5Multimedia_DIR "${_qt5Dir}/lib/cmake/Qt5Multimedia")
  set(Qt5LinguistTools_DIR "${_qt5Dir}/lib/cmake/Qt5LinguistTools")
  set(Qt5Test_DIR "${_qt5Dir}/lib/cmake/Qt5Test")
  set(QT_INCLUDES)
  set(QT_DEFINITIONS)
  set(QT_LIBRARIES)
  foreach (_qt5Component Core Gui Widgets Network Xml Multimedia LinguistTools Test)
    find_package("Qt5${_qt5Component}")
    set(QT_INCLUDES ${QT_INCLUDES} ${Qt5${_qt5Component}_INCLUDE_DIRS})
    set(QT_DEFINITIONS ${QT_DEFINITIONS} ${Qt5${_qt5Component}_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5${_qt5Component}_LIBRARIES})
  endforeach ()
  set(QT_EXECUTABLE_COMPILE_FLAGS ${Qt5Core_EXECUTABLE_COMPILE_FLAGS})
  if (WITH_DBUS)
    set(Qt5DBus_DIR "${_qt5Dir}/lib/cmake/Qt5DBus")
    find_package("Qt5DBus")
    if (Qt5DBus_LIBRARIES)
      set(QT_INCLUDES ${QT_INCLUDES} ${Qt5DBus_INCLUDE_DIRS})
      set(QT_DEFINITIONS ${QT_DEFINITIONS} ${Qt5DBus_DEFINITIONS})
      set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5DBus_LIBRARIES})
      set(HAVE_QTDBUS 1)
    endif (Qt5DBus_LIBRARIES)
  endif (WITH_DBUS)
  set(QT_LIBRARY_DIR "${_qt5Dir}/lib")
  set(QT_INCLUDE_DIR "${_qt5Dir}/include")
  set(QT_LUPDATE_EXECUTABLE ${_qt5Dir}/bin/lupdate)
  set(QT_PLUGINS_DIR ${_qt5Dir}/plugins)

  macro(qt4_add_translation)
    qt5_add_translation(${ARGV})
  endmacro(qt4_add_translation)

  macro(qt4_wrap_cpp)
    qt5_wrap_cpp(${ARGV})
  endmacro(qt4_wrap_cpp)

  macro(qt4_add_resources)
    qt5_add_resources(${ARGV})
  endmacro(qt4_add_resources)

  message(STATUS "Found Qt5: ${_qt5_corelib_install_prefix} (found version \"${Qt5Core_VERSION_STRING}\")")
endif (_qt5Dir)

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

set(BASIC_LIBRARIES ${QT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


if (APPLE)
  set(BASIC_LIBRARIES ${BASIC_LIBRARIES} "-framework CoreServices")
endif (APPLE)

if (WIN32)
  link_directories($ENV{LIB})
endif (WIN32)

if (BUILD_SHARED_LIBS)
  add_definitions(-DKID3_SHARED)

  # the RPATH to be used when installing, but only if it's not a system directory
  # see http://www.cmake.org/Wiki/CMake_RPATH_handling
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${WITH_LIBDIR}" _isSystemDir)
  if ("${_isSystemDir}" STREQUAL "-1")
     set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${WITH_LIBDIR}")
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

### Check for libQtDBus
if (WITH_DBUS AND QT_QTDBUS_LIBRARY)
  set(HAVE_QTDBUS 1)
endif (WITH_DBUS AND QT_QTDBUS_LIBRARY)

### Check for mntent.h
include(CheckIncludeFile)
check_include_file("mntent.h" HAVE_MNTENT_H)

set(QT_DEFINITIONS ${QT_DEFINITIONS} -DQT_ASCII_CAST_WARNINGS -DQT_NO_CAST_TO_ASCII)


### Check for Phonon
set(PHONON_LIBRARY)
set(PHONON_INCLUDE_DIR)
if (WITH_PHONON)
  find_library(PHONON_LIBRARY NAMES phonon phonon4 HINTS ${QT_LIBRARY_DIR})
  find_path(PHONON_INCLUDE_DIR NAMES phonon/phononnamespace.h
    HINTS ${QT_INCLUDE_DIR} ${INCLUDE_INSTALL_DIR} ${QT_LIBRARY_DIR}
          ${QT_HEADERS_DIR}/phonon ${QT_LIBRARY_DIR}/phonon.framework/Headers)
  if (PHONON_LIBRARY AND PHONON_INCLUDE_DIR)
    message(STATUS "Phonon found: ${PHONON_LIBRARY}")
    set(HAVE_PHONON 1)
    set(BASIC_LIBRARIES ${BASIC_LIBRARIES} ${PHONON_LIBRARY})
  else (PHONON_LIBRARY AND PHONON_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find Phonon")
  endif (PHONON_LIBRARY AND PHONON_INCLUDE_DIR)
endif (WITH_PHONON)

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -D_REENTRANT")
endif (CMAKE_COMPILER_IS_GNUCC)
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef -Wcast-align -Wall -W -Wpointer-arith -fno-exceptions -fno-check-new -fno-common -D_REENTRANT")
  check_cxx_compiler_flag(-Woverloaded-virtual _HAVE_W_OVERLOADED_VIRTUAL)
  if(_HAVE_W_OVERLOADED_VIRTUAL)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
  endif(_HAVE_W_OVERLOADED_VIRTUAL)
endif (CMAKE_COMPILER_IS_GNUCXX)
if (MSVC)
  # Treat wchar_t as built-in type, else QString::fromWCharArray is unresolved
  add_definitions("/Zc:wchar_t-")
endif (MSVC)
if (APPLE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.5")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.5")
endif (APPLE)
add_definitions(${QT_DEFINITIONS} -DHAVE_CONFIG_H=1 -DQT_NO_EXCEPTIONS -DQT_NO_STL)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDES} ${PHONON_INCLUDE_DIR})

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


# To create a package, run cpack
if (APPLE)
  set(CPACK_BINARY_DRAGNDROP ON)
  set(CPACK_GENERATOR DragNDrop)
elseif (WIN32)
  set(CPACK_GENERATOR ZIP)
else (APPLE)
  set(CPACK_GENERATOR DEB)
endif (APPLE)

if (BUILD_KDE_APP OR APPLE OR WIN32)
  set(CPACK_PACKAGE_NAME "kid3")
  set(CPACK_DEBIAN_PACKAGE_SECTION "kde")
else (BUILD_KDE_APP OR APPLE OR WIN32)
  set(CPACK_PACKAGE_NAME "kid3-qt")
  set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
endif (BUILD_KDE_APP OR APPLE OR WIN32)
set(CPACK_PACKAGE_CONTACT "ufleisch@users.sourceforge.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Audio tag editor")
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://kid3.sourceforge.net/")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
