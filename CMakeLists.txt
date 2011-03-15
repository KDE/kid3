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
# cmake -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= -D WITH_TUNEPIMP=OFF ..\kid3
# mingw32-make
# cpack
#
# Mac OS X:
# cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX= -D WITH_TUNEPIMP=OFF ../kid3
# make
# cpack

project(kid3)

set(WITH_TAGLIB ON CACHE BOOL "build with TagLib")
set(WITH_MP4V2 ON CACHE BOOL "build with mp4v2")
set(WITH_ID3LIB ON CACHE BOOL "build with id3lib")
set(WITH_VORBIS ON CACHE BOOL "build with Ogg/Vorbis")
set(WITH_FLAC ON CACHE BOOL "build with FLAC")
set(WITH_TUNEPIMP ON CACHE BOOL "build with TunePimp")
set(WITH_GCC_PCH OFF CACHE BOOL "enable precompiled headers")
if (APPLE OR WIN32)
  set(WITH_KDE OFF CACHE BOOL "build with KDE libraries")
else (APPLE OR WIN32)
  set(WITH_KDE ON CACHE BOOL "build with KDE libraries")
endif (APPLE OR WIN32)

if (WITH_GCC_PCH)
include(GccPchMacros.cmake)
endif (WITH_GCC_PCH)

if (WITH_KDE)

find_package(KDE4 REQUIRED)
find_package(Phonon)
include (KDE4Defaults)

else (WITH_KDE)

find_package(Threads)

# Platform specific default installation paths
if (APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_DOCDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_TRANSLATIONSDIR_DEFAULT "kid3.app/Contents/Resources")
  set(WITH_BINDIR_DEFAULT ".")
elseif (WIN32)
  set(WITH_DATAROOTDIR_DEFAULT ".")
  set(WITH_DOCDIR_DEFAULT ".")
  set(WITH_TRANSLATIONSDIR_DEFAULT ".")
  set(WITH_BINDIR_DEFAULT ".")
else (APPLE)
  set(WITH_DATAROOTDIR_DEFAULT "share")
  set(WITH_DOCDIR_DEFAULT "share/doc/kid3-qt")
  set(WITH_TRANSLATIONSDIR_DEFAULT "share/kid3-qt/translations")
  set(WITH_BINDIR_DEFAULT "bin")
endif (APPLE)

set(WITH_DBUS ON CACHE BOOL "build with QtDBus")
set(WITH_PHONON ON CACHE BOOL "build with Phonon")
set(WITH_DATAROOTDIR ${WITH_DATAROOTDIR_DEFAULT} CACHE STRING
    "data root directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_DOCDIR ${WITH_DOCDIR_DEFAULT} CACHE STRING
    "documentation directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_TRANSLATIONSDIR ${WITH_TRANSLATIONSDIR_DEFAULT} CACHE STRING
    "translations directory relative to CMAKE_INSTALL_PREFIX")
set(WITH_BINDIR ${WITH_BINDIR_DEFAULT} CACHE STRING
    "binary directory relative to CMAKE_INSTALL_PREFIX")
if (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "${CMAKE_INSTALL_PREFIX}/")
else (CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX_SLASH "")
endif (CMAKE_INSTALL_PREFIX)

set(CFG_DATAROOTDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DATAROOTDIR})
set(CFG_DOCDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_DOCDIR})
set(CFG_TRANSLATIONSDIR ${CMAKE_INSTALL_PREFIX_SLASH}${WITH_TRANSLATIONSDIR})

cmake_minimum_required(VERSION 2.8)
set(_QT4_COMPONENTS QtCore QtGui QtNetwork QtXml)
if(WITH_PHONON)
  list(APPEND _QT4_COMPONENTS phonon)
endif(WITH_PHONON)
if (WIN32)
  set(QT_USE_QTMAIN TRUE)
endif (WIN32)
if (NOT WITH_DBUS)
  set(QT_USE_QTDBUS OFF)
endif (NOT WITH_DBUS)
find_package(Qt4 COMPONENTS ${_QT4_COMPONENTS} REQUIRED)
include (${QT_USE_FILE})
include (CheckCXXSourceCompiles)
find_package(Perl REQUIRED)

### Check for xsltproc
find_program(XSLTPROC xsltproc DOC "xsltproc transforms XML via XSLT"
  PATHS $ENV{XSLTPROCDIR})
if (NOT XSLTPROC)
  message(FATAL_ERROR "Could not find xsltproc")
endif (NOT XSLTPROC)

### Check for HTML docbook.xsl
find_path(DOCBOOK_XSL_DIR html/docbook.xsl
  PATHS /usr/share/xml/docbook/stylesheet/nwalsh
        /usr/share/apps/ksgmltools2/docbook/xsl
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
  set(BASIC_LIBRARIES ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${PHONON_LIBS})
else (WITH_KDE)
  set(BASIC_LIBRARIES ${QT_LIBRARIES})
endif (WITH_KDE)

if (WIN32)
link_directories($ENV{LIB})
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
      endif(NOT ${TAGLIB_VERSION} VERSION_LESS 1.4)
    endif(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS AND TAGLIB_VERSION)
  endif(TAGLIBCONFIG_EXECUTABLE)
  if (NOT HAVE_TAGLIB)
    message(FATAL_ERROR "Could not find Taglib")
  endif (NOT HAVE_TAGLIB)
  endif (WIN32)
endif(WITH_TAGLIB)

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
    if (WIN32)
      find_library(_ZLIB_PATH_NAME z)
      if (_ZLIB_PATH_NAME)
        get_filename_component(_ZLIB_PATH ${_ZLIB_PATH_NAME} PATH)
      else (_ZLIB_PATH_NAME)
        message(FATAL_ERROR "Could not find zlib")
      endif (_ZLIB_PATH_NAME)
      set(ID3LIB_DEFINITIONS -DID3LIB_LINKOPTION=1)
      set(ID3LIB_LIBRARIES ${ID3LIB_LIBRARIES} "-L${_ZLIB_PATH}" -lz -lstdc++)
    elseif (APPLE)
      set(ID3LIB_LIBRARIES ${ID3LIB_LIBRARIES} -lz -liconv)
    endif (WIN32)
    set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${ID3LIB_LIBRARIES} -lz -lstdc++)
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

### Check for TunePimp
set(TUNEPIMP_LIBRARIES)
set(HAVE_TUNEPIMP 0)
if(WITH_TUNEPIMP)
  find_path(TUNEPIMP_INCLUDE_DIR tunepimp/tp_c.h)
  if(NOT TUNEPIMP_INCLUDE_DIR)
    find_path(TUNEPIMP_INCLUDE_DIR tunepimp-0.5/tp_c.h)
  endif(NOT TUNEPIMP_INCLUDE_DIR)
  find_library(TUNEPIMP_LIBRARIES NAMES tunepimp)
  if(TUNEPIMP_INCLUDE_DIR AND TUNEPIMP_LIBRARIES)
    message(STATUS "TunePimp found: ${TUNEPIMP_LIBRARIES}")
    CHECK_LIBRARY_EXISTS(${TUNEPIMP_LIBRARIES} tp_SetFileNameEncoding "" TUNEPIMP_FOUND_VERSION_4)
    CHECK_LIBRARY_EXISTS(${TUNEPIMP_LIBRARIES} tr_GetPUID "" TUNEPIMP_FOUND_VERSION_5)
    if(TUNEPIMP_FOUND_VERSION_5)
      set(HAVE_TUNEPIMP 5)
    elseif(TUNEPIMP_FOUND_VERSION_4)
      set(HAVE_TUNEPIMP 4)
    else(TUNEPIMP_FOUND_VERSION_5)
      set(HAVE_TUNEPIMP 3)
    endif(TUNEPIMP_FOUND_VERSION_5)
  else(TUNEPIMP_INCLUDE_DIR AND TUNEPIMP_LIBRARIES)
    message(FATAL_ERROR "Could not find TunePimp")
  endif(TUNEPIMP_INCLUDE_DIR AND TUNEPIMP_LIBRARIES)
endif(WITH_TUNEPIMP)

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

if (WITH_KDE)

### Check for Phonon
if(PHONON_FOUND)
  set(HAVE_PHONON 1)
endif(PHONON_FOUND)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${TAGLIB_CFLAGS} -DHAVE_CONFIG_H=1 -DKDE_NO_COMPAT)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} ${PHONON_INCLUDES} ${MP4V2_INCLUDE_DIR} ${ID3LIB_INCLUDE_DIR} ${TUNEPIMP_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR} ${FLACPP_INCLUDE_DIR})

else (WITH_KDE)

### Check for Phonon
if (QT_USE_PHONON AND QT_PHONON_FOUND)
  set(HAVE_PHONON 1)
  if (APPLE)
    set(BASIC_LIBRARIES ${BASIC_LIBRARIES} -lgcc)
  endif (APPLE)
endif (QT_USE_PHONON AND QT_PHONON_FOUND)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D_REENTRANT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -D_REENTRANT")
add_definitions(${QT_DEFINITIONS} ${TAGLIB_CFLAGS} ${ID3LIB_DEFINITIONS} ${FLAC_DEFINITIONS} -DHAVE_CONFIG_H=1)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDES} ${MP4V2_INCLUDE_DIR} ${ID3LIB_INCLUDE_DIR} ${TUNEPIMP_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR} ${FLACPP_INCLUDE_DIR})

if (UNIX AND NOT APPLE)
  set(KID3_EXECUTABLE kid3-qt)
else (UNIX AND NOT APPLE)
  set(KID3_EXECUTABLE kid3)
endif (UNIX AND NOT APPLE)

endif (WITH_KDE)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(kid3)
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
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 6)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_CONTACT "ufleisch@users.sourceforge.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Audio tag editor")
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://kid3.sourceforge.net/")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)