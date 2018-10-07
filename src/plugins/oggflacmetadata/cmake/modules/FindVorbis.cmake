if(VORBIS_INCLUDE_DIR)
  set(VORBIS_FIND_QUIETLY TRUE)
endif()

find_package(Ogg QUIET)

find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h)
find_library(VORBIS_LIBRARY NAMES vorbis libvorbis_static)
find_library(VORBISFILE_LIBRARY NAMES vorbisfile libvorbisfile_static)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vorbis REQUIRED_VARS VORBIS_LIBRARY VORBISFILE_LIBRARY VORBIS_INCLUDE_DIR OGG_FOUND)

if(VORBIS_FOUND)
  set(HAVE_VORBIS 1)
  if(NOT TARGET Vorbis::Vorbis)
    add_library(Vorbis::Vorbis UNKNOWN IMPORTED)
    set_target_properties(Vorbis::Vorbis PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${VORBIS_INCLUDE_DIR}"
      IMPORTED_LOCATION "${VORBIS_LIBRARY}"
      INTERFACE_LINK_LIBRARIES Ogg::Ogg
    )
  endif()
  if(NOT TARGET VorbisFile::VorbisFile)
    add_library(VorbisFile::VorbisFile UNKNOWN IMPORTED)
    set_target_properties(VorbisFile::VorbisFile PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${VORBIS_INCLUDE_DIR}"
      IMPORTED_LOCATION "${VORBISFILE_LIBRARY}"
      INTERFACE_LINK_LIBRARIES "Ogg::Ogg;Vorbis::Vorbis"
    )
  endif()
endif()

mark_as_advanced(VORBIS_INCLUDE_DIR VORBIS_LIBRARY VORBISFILE_LIBRARY)
