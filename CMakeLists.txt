# Build Kid3 using CMake
#
# Linux:
# cmake -D CMAKE_BUILD_TYPE=Release -D WITH_KDE=OFF -D CMAKE_INSTALL_PREFIX=/usr ../kid3
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

set(WITH_TAGLIB ON CACHE BOOL "build with TagLib")
set(WITH_MP4V2 OFF CACHE BOOL "build with mp4v2")
set(WITH_ID3LIB ON CACHE BOOL "build with id3lib")
set(WITH_VORBIS ON CACHE BOOL "build with Ogg/Vorbis")
set(WITH_FLAC ON CACHE BOOL "build with FLAC")
set(WITH_CHROMAPRINT ON CACHE BOOL "build with Chromaprint")
set(WITH_FFMPEG OFF CACHE BOOL "force use of FFmpeg for Chromaprint decoding")
set(WITH_GSTREAMER OFF CACHE BOOL "force use of GStreamer for Chromaprint decoding")
set(WITH_DBUS ON CACHE BOOL "build with QtDBus")
set(WITH_PHONON ON CACHE BOOL "build with Phonon")
set(WITH_GCC_PCH OFF CACHE BOOL "enable precompiled headers")
if (APPLE OR WIN32)
  set(WITH_KDE OFF CACHE BOOL "build with KDE libraries")
else (APPLE OR WIN32)
  set(WITH_KDE ON CACHE BOOL "build with KDE libraries")
endif (APPLE OR WIN32)

# Version information
set(CPACK_PACKAGE_VERSION_MAJOR 2)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
#set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "git20120926")
set(RELEASE_YEAR 2012)

if (WITH_GCC_PCH)
include(GccPchMacros.cmake)
endif (WITH_GCC_PCH)

if (WITH_KDE)

find_package(KDE4 REQUIRED)
if (WITH_PHONON)
  find_package(Phonon)
endif (WITH_PHONON)
include (KDE4Defaults)

# Set variables used to install shared libraries
set(WITH_BINDIR ${BIN_INSTALL_DIR})
set(WITH_LIBDIR ${LIB_INSTALL_DIR})

else (WITH_KDE)

find_package(Threads)

# Platform specific default installation paths
if (APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_DOCDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_TRANSLATIONSDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT ".")
elseif (WIN32)
  set(WITH_DATAROOTDIR_DEFAULT ".")
  set(WITH_DOCDIR_DEFAULT ".")
  set(WITH_TRANSLATIONSDIR_DEFAULT ".")
  set(WITH_BINDIR_DEFAULT ".")
  set(WITH_LIBDIR_DEFAULT ".")
else (APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "share")
  set(WITH_DOCDIR_DEFAULT "share/doc/kid3-qt")
  set(WITH_TRANSLATIONSDIR_DEFAULT "share/kid3-qt/translations")
  set(WITH_BINDIR_DEFAULT "bin")
  set(WITH_LIBDIR_DEFAULT "lib")
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
if (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "${CMAKE_INSTALL_PREFIX}/")
else (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "")
endif (CMAKE_INSTALL_PREFIX)

set(CFG_DATAROOTDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DATAROOTDIR})
set(CFG_DOCDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DOCDIR})
set(CFG_TRANSLATIONSDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_TRANSLATIONSDIR})

cmake_minimum_required(VERSION 2.8)
string(REGEX MATCH "^.*qt5" _qt5Dir "${QT_QMAKE_EXECUTABLE}")
if (_qt5Dir)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(Qt5Core_DIR "${_qt5Dir}/lib/cmake/Qt5Core")
  set(Qt5Gui_DIR "${_qt5Dir}/lib/cmake/Qt5Gui")
  set(Qt5Widgets_DIR "${_qt5Dir}/lib/cmake/Qt5Widgets")
  set(Qt5Network_DIR "${_qt5Dir}/lib/cmake/Qt5Network")
  set(Qt5Xml_DIR "${_qt5Dir}/lib/cmake/Qt5Xml")
  set(Qt5LinguistTools_DIR "${_qt5Dir}/lib/cmake/Qt5LinguistTools")
  set(QT_INCLUDES)
  set(QT_DEFINITIONS)
  set(QT_LIBRARIES)
  foreach (_qt5Component Core Gui Widgets Network Xml LinguistTools)
    find_package("Qt5${_qt5Component}")
    set(QT_INCLUDES ${QT_INCLUDES} ${Qt5${_qt5Component}_INCLUDE_DIRS})
    set(QT_DEFINITIONS ${QT_DEFINITIONS} ${Qt5${_qt5Component}_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5${_qt5Component}_LIBRARIES})
  endforeach ()
  set(QT_LIBRARY_DIR "${_qt5Dir}/lib")
  set(QT_INCLUDE_DIR "${_qt5Dir}/include")

  macro(qt4_add_translation)
    qt5_add_translation(${ARGV})
  endmacro(qt4_add_translation)

  macro(qt4_wrap_cpp)
    qt5_wrap_cpp(${ARGV})
  endmacro(qt4_wrap_cpp)

  macro(qt4_add_resources)
    qt5_add_resources(${ARGV})
  endmacro(qt4_add_resources)
else (_qt5Dir)
  set(_QT4_COMPONENTS QtCore QtGui QtNetwork QtXml)
  if (WIN32)
    set(QT_USE_QTMAIN TRUE)
  endif (WIN32)
  set(QT_USE_QTDBUS ${WITH_DBUS})
  find_package(Qt4 COMPONENTS ${_QT4_COMPONENTS} REQUIRED)
  include (${QT_USE_FILE})
endif (_qt5Dir)
include (CheckCXXSourceCompiles)
find_package(Perl REQUIRED)

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

endif (WITH_KDE)

include (CheckLibraryExists)

if (WITH_KDE)
  set(CONFIG_USE_KDE 1)
  set(BASIC_LIBRARIES ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
  if (WITH_PHONON)
    set(BASIC_LIBRARIES ${BASIC_LIBRARIES} ${PHONON_LIBS})
  endif (WITH_PHONON)
else (WITH_KDE)
  set(BASIC_LIBRARIES ${QT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  if (APPLE)
    set(BASIC_LIBRARIES ${BASIC_LIBRARIES} "-framework CoreServices")
  endif (APPLE)
endif (WITH_KDE)

if (WIN32)
link_directories($ENV{LIB})
endif (WIN32)

if (BUILD_SHARED_LIBS)
  add_definitions(-DKID3_SHARED)
endif (BUILD_SHARED_LIBS)

### Check for zlib
if (WIN32)
  find_library(_ZLIB_PATH_NAME z)
  if (_ZLIB_PATH_NAME)
    get_filename_component(_ZLIB_PATH ${_ZLIB_PATH_NAME} PATH)
  else (_ZLIB_PATH_NAME)
    message(FATAL_ERROR "Could not find zlib")
  endif (_ZLIB_PATH_NAME)
  set(ZLIB_LIBRARIES "-L${_ZLIB_PATH}" -lz)
else (WIN32)
  set(ZLIB_LIBRARIES -lz)
endif (WIN32)

### Check for taglib
set(TAGLIB_LIBRARIES)
set(TAGLIB_CFLAGS)
if(WITH_TAGLIB)
  if (WIN32)
  find_library(TAGLIB_LIBRARIES NAMES tag)
  find_path(TAGLIB_INCLUDE_DIR taglib.h PATHS $ENV{INCLUDE}/taglib)
  if (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIR)
    set(TAGLIB_CFLAGS -I${TAGLIB_INCLUDE_DIR} -DTAGLIB_STATIC)
    set(HAVE_TAGLIB 1)
    set(TAGLIB_LIBRARIES ${TAGLIB_LIBRARIES} ${ZLIB_LIBRARIES})
  endif (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIR)
  else (WIN32)
  find_program(TAGLIBCONFIG_EXECUTABLE NAMES taglib-config PATHS /usr/bin /usr/local/bin ${BIN_INSTALL_DIR})
  if(TAGLIBCONFIG_EXECUTABLE)
    exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_LIBRARIES)
    exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_CFLAGS)
    exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_VERSION)
    if(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS AND TAGLIB_VERSION)
      if(NOT ${TAGLIB_VERSION} VERSION_LESS 1.4)
        set(HAVE_TAGLIB 1)
        message(STATUS "TagLib found: ${TAGLIB_LIBRARIES}")
        set(TAGLIB_LIBRARIES ${TAGLIB_LIBRARIES} ${ZLIB_LIBRARIES})
      endif(NOT ${TAGLIB_VERSION} VERSION_LESS 1.4)
    endif(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS AND TAGLIB_VERSION)
  endif(TAGLIBCONFIG_EXECUTABLE)
  if (NOT HAVE_TAGLIB)
    message(FATAL_ERROR "Could not find Taglib")
  endif (NOT HAVE_TAGLIB)
  endif (WIN32)
endif(WITH_TAGLIB)
if (TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
  set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${TAGLIB_LIBRARIES})
  set(_CMAKE_REQUIRED_DEFINITIONS_TMP ${CMAKE_REQUIRED_DEFINITIONS})
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${TAGLIB_CFLAGS})
  CHECK_CXX_SOURCE_COMPILES("#include <mpegfile.h>\nint main() {\n  TagLib::MPEG::File file(\"somefile.mp3\");\n  file.save(3, false, 3);\n  return 0;\n}\n" HAVE_TAGLIB_ID3V23_SUPPORT)
  set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
  set(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_TMP})
endif (TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)

### Check for mp4v2
set(MP4V2_LIBRARIES)
if(WITH_MP4V2)
  find_path(MP4V2_INCLUDE_DIR mp4.h
   /usr/include/
   /usr/local/include/
  )

  find_path(MP4V2_MP4V2_INCLUDE_DIR mp4v2/mp4v2.h
   /usr/include/
   /usr/local/include/
  )

  find_library(MP4V2_LIBRARIES NAMES mp4v2
   PATHS
   /usr/lib
   /usr/local/lib
  )

  if (MP4V2_MP4V2_INCLUDE_DIR)
    set(MP4V2_INCLUDE_DIR ${MP4V2_MP4V2_INCLUDE_DIR})
    set(HAVE_MP4V2_MP4V2_H 1)
  endif (MP4V2_MP4V2_INCLUDE_DIR)

  if (MP4V2_INCLUDE_DIR AND MP4V2_LIBRARIES)
    message(STATUS "mp4v2 found: ${MP4V2_LIBRARIES}")
    set(HAVE_MP4V2 1)
    set(MP4V2_FOUND TRUE)

    set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${MP4V2_LIBRARIES})
    set(_CMAKE_REQUIRED_INCLUDES_TMP ${CMAKE_REQUIRED_INCLUDES})
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MP4V2_INCLUDE_DIR})
    if (MP4V2_MP4V2_INCLUDE_DIR)
      CHECK_CXX_SOURCE_COMPILES("#include <mp4v2/mp4v2.h>\nint main() {\n  MP4FileHandle hFile;\n  uint32_t index;\n  char* ppName;\n  uint8_t* ppValue;\n  uint32_t pValueSize;\n  MP4GetMetadataByIndex(hFile, index, &ppName, &ppValue, &pValueSize);\n  return 0;\n}\n" HAVE_MP4V2_MP4GETMETADATABYINDEX_CHARPP_ARG)
    else (MP4V2_MP4V2_INCLUDE_DIR)
      CHECK_CXX_SOURCE_COMPILES("#include <mp4.h>\nint main() {\n  MP4FileHandle hFile;\n  u_int32_t index;\n  char* ppName;\n  u_int8_t* ppValue;\n  u_int32_t pValueSize;\n  MP4GetMetadataByIndex(hFile, index, &ppName, &ppValue, &pValueSize);\n  return 0;\n}\n" HAVE_MP4V2_MP4GETMETADATABYINDEX_CHARPP_ARG)
    endif (MP4V2_MP4V2_INCLUDE_DIR)
    set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
    set(CMAKE_REQUIRED_INCLUDES ${_CMAKE_REQUIRED_INCLUDES_TMP})

  else (MP4V2_INCLUDE_DIR AND MP4V2_LIBRARIES)
    message(STATUS "Could not find mp4v2")
  endif (MP4V2_INCLUDE_DIR AND MP4V2_LIBRARIES)
endif(WITH_MP4V2)

### Check for id3lib
set(ID3LIB_LIBRARIES)
set(ID3LIB_DEFINITIONS)
if(WITH_ID3LIB)
  find_path(ID3LIB_INCLUDE_DIR id3/tag.h)
  find_library(ID3LIB_LIBRARIES NAMES id3)
  if(ID3LIB_INCLUDE_DIR AND ID3LIB_LIBRARIES)
    message(STATUS "id3lib found: ${ID3LIB_LIBRARIES}")
    set(HAVE_ID3LIB 1)
    set(ID3LIB_LIBRARIES ${ID3LIB_LIBRARIES} ${ZLIB_LIBRARIES} -lstdc++)
    if (WIN32)
      set(ID3LIB_DEFINITIONS -DID3LIB_LINKOPTION=1)
    elseif (APPLE)
      set(ID3LIB_LIBRARIES ${ID3LIB_LIBRARIES} -liconv)
    endif (WIN32)
    set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${ID3LIB_LIBRARIES})
    set(_CMAKE_REQUIRED_INCLUDES_TMP ${CMAKE_REQUIRED_INCLUDES})
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${ID3LIB_INCLUDE_DIR})
    set(_CMAKE_REQUIRED_DEFINITIONS_TMP ${CMAKE_REQUIRED_DEFINITIONS})
    set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${ID3LIB_DEFINITIONS})
    CHECK_CXX_SOURCE_COMPILES("#include <id3/globals.h>\nint main() {\n  Mp3_Headerinfo info;\n  info.vbr_bitrate = 0;\n  return 0;\n}\n" ID3LIB_VBR_SUPPORT_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
    set(CMAKE_REQUIRED_INCLUDES ${_CMAKE_REQUIRED_INCLUDES_TMP})
    set(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_TMP})

    if(NOT ID3LIB_VBR_SUPPORT_FOUND)
      set(HAVE_NO_ID3LIB_VBR 1)
    endif(NOT ID3LIB_VBR_SUPPORT_FOUND)
  else(ID3LIB_INCLUDE_DIR AND ID3LIB_LIBRARIES)
    message(FATAL_ERROR "Could not find id3lib")
  endif(ID3LIB_INCLUDE_DIR AND ID3LIB_LIBRARIES)
endif(WITH_ID3LIB)

### Check for Chromaprint
set(CHROMAPRINT_INCLUDES)
set(CHROMAPRINT_LIBS)
set(CHROMAPRINT_CFLAGS)
if(WITH_CHROMAPRINT)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
  find_package(Chromaprint REQUIRED)
  if(CHROMAPRINT_INCLUDE_DIR AND CHROMAPRINT_LIBRARIES)
    set(HAVE_CHROMAPRINT 1)
  endif(CHROMAPRINT_INCLUDE_DIR AND CHROMAPRINT_LIBRARIES)
  set(CHROMAPRINT_INCLUDES ${CHROMAPRINT_INCLUDE_DIR})
  set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBRARIES})
  if(WITH_FFMPEG)
    find_package(FFmpeg REQUIRED)
  elseif(WITH_GSTREAMER)
    find_package(PkgConfig)
    pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10)
  else(WITH_FFMPEG)
    # Neither FFmpeg nor GStreamer is forced, select FFmpeg if Chromaprint
    # depends on it or if on Windows or Mac OS X.
    include(GetPrerequisites)
    get_prerequisites(${CHROMAPRINT_LIBRARIES} _chromaprintDeps 0 0 "" "")
    string(FIND "${_chromaprintDeps}" "libavcodec" _avPos)
    if(${_avPos} GREATER -1 OR APPLE OR WIN32)
      message(STATUS "FFmpeg selected for decoding, use WITH_GSTREAMER=ON to use GStreamer instead.")
      find_package(FFmpeg REQUIRED)
    else()
      message(STATUS "GStreamer selected for decoding, use WITH_FFMPEG=ON to use FFmpeg instead.")
      find_package(PkgConfig)
      pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10)
    endif()
  endif(WITH_FFMPEG)
  if(GSTREAMER_FOUND)
    set(HAVE_GSTREAMER 1)
    set(CHROMAPRINT_CFLAGS ${GSTREAMER_CFLAGS_OTHER})
    set(CHROMAPRINT_INCLUDES ${CHROMAPRINT_INCLUDES} ${GSTREAMER_INCLUDE_DIRS})
    set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${GSTREAMER_LDFLAGS})
  else(GSTREAMER_FOUND)
    include(CheckFunctionExists)
    set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_LIBRARIES
      ${FFMPEG_LIBAVFORMAT_LIBRARIES}
      ${FFMPEG_LIBAVCODEC_LIBRARIES}
      ${FFMPEG_LIBAVUTIL_LIBRARIES})
    check_function_exists(av_audio_convert HAVE_AV_AUDIO_CONVERT)
    set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})

    set(CHROMAPRINT_INCLUDES ${CHROMAPRINT_INCLUDES} ${FFMPEG_LIBAVCODEC_INCLUDE_DIRS} ${FFMPEG_LIBAVFORMAT_INCLUDE_DIRS} ${FFMPEG_LIBAVUTIL_INCLUDE_DIRS})
    set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${FFMPEG_LIBAVFORMAT_LIBRARIES} ${FFMPEG_LIBAVCODEC_LIBRARIES} ${FFMPEG_LIBAVUTIL_LIBRARIES})
  endif(GSTREAMER_FOUND)
  if (APPLE)
    find_library(ACCELERATE_LIBRARIES Accelerate)
    set(CHROMAPRINT_LIBS ${CHROMAPRINT_LIBS} ${ACCELERATE_LIBRARIES})
  endif (APPLE)
endif(WITH_CHROMAPRINT)

### Check for Vorbis
set(VORBIS_LIBRARIES)
if(WITH_VORBIS)
  find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h)
  find_library(OGG_LIBRARY NAMES ogg)
  find_library(VORBIS_LIBRARY NAMES vorbis)
  find_library(VORBISFILE_LIBRARY NAMES vorbisfile)
  if(VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)
    set(VORBIS_LIBRARIES ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY})
    message(STATUS "Vorbis found: ${VORBIS_LIBRARIES}")
    set(HAVE_VORBIS 1)
  else(VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)
    message(FATAL_ERROR "Could not find Vorbis")
  endif(VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)
endif(WITH_VORBIS)

### Check for FLAC++
set(FLAC_LIBRARIES)
set(FLAC_DEFINITIONS)
if(WITH_FLAC)
  find_path(FLACPP_INCLUDE_DIR FLAC++/metadata.h)
  find_library(FLAC_LIBRARY NAMES FLAC)
  find_library(FLACPP_LIBRARY NAMES FLAC++)
  if(FLACPP_INCLUDE_DIR AND FLAC_LIBRARY AND FLACPP_LIBRARY)
    set(FLAC_LIBRARIES ${FLACPP_LIBRARY} ${FLAC_LIBRARY})
    message(STATUS "FLAC++ found: ${FLAC_LIBRARIES}")
    set(HAVE_FLAC 1)
    if (WIN32)
      set(FLAC_DEFINITIONS -DFLAC__NO_DLL)
      set(FLAC_LIBRARIES ${FLAC_LIBRARIES} ws2_32)
    endif (WIN32)
    set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${FLAC_LIBRARIES} ${OGG_LIBRARY})
    set(_CMAKE_REQUIRED_INCLUDES_TMP ${CMAKE_REQUIRED_INCLUDES})
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${FLACPP_INCLUDE_DIR})
    set(_CMAKE_REQUIRED_DEFINITIONS_TMP ${CMAKE_REQUIRED_DEFINITIONS})
    set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${FLAC_DEFINITIONS})
    CHECK_CXX_SOURCE_COMPILES("#include <FLAC++/metadata.h>\nint main() {\n  FLAC::Metadata::VorbisComment vc;\n  const ::FLAC__StreamMetadata* fsmd = vc;\n  return 0;\n}\n" FLAC_STREAMMETADATA_OPERATOR_FOUND)
    CHECK_CXX_SOURCE_COMPILES("#include <FLAC++/metadata.h>\nint main() {\n  FLAC::Metadata::Picture pic;\n  return 0;\n}\n" FLAC_METADATA_PICTURE_FOUND)
    set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
    set(CMAKE_REQUIRED_INCLUDES ${_CMAKE_REQUIRED_INCLUDES_TMP})
    set(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_TMP})

    if(NOT FLAC_STREAMMETADATA_OPERATOR_FOUND)
      set(HAVE_NO_FLAC_STREAMMETADATA_OPERATOR 1)
    endif(NOT FLAC_STREAMMETADATA_OPERATOR_FOUND)
    if(FLAC_METADATA_PICTURE_FOUND)
      set(HAVE_FLAC_PICTURE 1)
    endif(FLAC_METADATA_PICTURE_FOUND)
  else(FLACPP_INCLUDE_DIR AND FLAC_LIBRARY AND FLACPP_LIBRARY)
    message(FATAL_ERROR "Could not find FLAC++")
  endif(FLACPP_INCLUDE_DIR AND FLAC_LIBRARY AND FLACPP_LIBRARY)
endif(WITH_FLAC)

### Check for libQtDBus
if (WITH_DBUS AND QT_QTDBUS_LIBRARY)
  set(HAVE_QTDBUS 1)
endif (WITH_DBUS AND QT_QTDBUS_LIBRARY)

### Check for mntent.h
include(CheckIncludeFile)
check_include_file("mntent.h" HAVE_MNTENT_H)

if (WITH_KDE)

### Check for Phonon
if (WITH_PHONON AND PHONON_FOUND)
  set(HAVE_PHONON 1)
endif (WITH_PHONON AND PHONON_FOUND)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${TAGLIB_CFLAGS} ${CHROMAPRINT_CFLAGS} -DHAVE_CONFIG_H=1 -DKDE_NO_COMPAT)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} ${PHONON_INCLUDES} ${MP4V2_INCLUDE_DIR} ${ID3LIB_INCLUDE_DIR} ${CHROMAPRINT_INCLUDES} ${VORBIS_INCLUDE_DIR} ${FLACPP_INCLUDE_DIR})

else (WITH_KDE)

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
    if (APPLE)
      set(BASIC_LIBRARIES ${BASIC_LIBRARIES} -lgcc)
    endif (APPLE)
  else (PHONON_LIBRARY AND PHONON_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find Phonon")
  endif (PHONON_LIBRARY AND PHONON_INCLUDE_DIR)
endif (WITH_PHONON)

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D_REENTRANT")
endif (CMAKE_COMPILER_IS_GNUCC)
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-exceptions -D_REENTRANT")
endif (CMAKE_COMPILER_IS_GNUCXX)
add_definitions(${QT_DEFINITIONS} ${TAGLIB_CFLAGS} ${CHROMAPRINT_CFLAGS} ${ID3LIB_DEFINITIONS} ${FLAC_DEFINITIONS} -DHAVE_CONFIG_H=1 -DQT_NO_EXCEPTIONS -DQT_NO_STL)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDES} ${PHONON_INCLUDE_DIR} ${MP4V2_INCLUDE_DIR} ${ID3LIB_INCLUDE_DIR} ${CHROMAPRINT_INCLUDES} ${VORBIS_INCLUDE_DIR} ${FLACPP_INCLUDE_DIR})

if (NOT WIN32)
  include (CheckCXXCompilerFlag)
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

endif (WITH_KDE)

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

if (WITH_KDE OR APPLE OR WIN32)
  set(CPACK_PACKAGE_NAME "kid3")
  set(CPACK_DEBIAN_PACKAGE_SECTION "kde")
else (WITH_KDE OR APPLE OR WIN32)
  set(CPACK_PACKAGE_NAME "kid3-qt")
  set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
endif (WITH_KDE OR APPLE OR WIN32)
set(CPACK_PACKAGE_CONTACT "ufleisch@users.sourceforge.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Audio tag editor")
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://kid3.sourceforge.net/")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
