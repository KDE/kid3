if(OGG_INCLUDE_DIR)
  set(OGG_FIND_QUIETLY TRUE)
endif()

find_path(OGG_INCLUDE_DIR ogg/ogg.h)
find_library(OGG_LIBRARY NAMES ogg libogg_static)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ogg REQUIRED_VARS OGG_LIBRARY OGG_INCLUDE_DIR)

if(OGG_FOUND)
  if(NOT TARGET Ogg::Ogg)
    add_library(Ogg::Ogg UNKNOWN IMPORTED)
    set_target_properties(Ogg::Ogg PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OGG_INCLUDE_DIR}"
      IMPORTED_LOCATION "${OGG_LIBRARY}"
    )
  endif()
endif()

mark_as_advanced(OGG_INCLUDE_DIR OGG_LIBRARY)
