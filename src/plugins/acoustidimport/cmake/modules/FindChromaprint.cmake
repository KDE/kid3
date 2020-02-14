# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
include(FindPackageHandleStandardArgs)
find_package(PkgConfig)
pkg_check_modules(PKG_LIBCHROMAPRINT libchromaprint)

find_path(CHROMAPRINT_INCLUDE_DIR chromaprint.h
  ${PKG_LIBCHROMAPRINT_INCLUDE_DIRS}
  /usr/include
  /usr/local/include)

find_library(CHROMAPRINT_LIBRARIES
  NAMES chromaprint chromaprint.dll
  PATHS ${PKG_LIBCHROMAPRINT_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib)

find_package_handle_standard_args(Chromaprint DEFAULT_MSG CHROMAPRINT_LIBRARIES CHROMAPRINT_INCLUDE_DIR)

if(CHROMAPRINT_FOUND)
  if(NOT TARGET Chromaprint::Chromaprint)
    if(WIN32)
      set(CHROMAPRINT_COMPILE_DEFINITIONS CHROMAPRINT_NODLL)
    endif()
    add_library(Chromaprint::Chromaprint UNKNOWN IMPORTED)
    set_target_properties(Chromaprint::Chromaprint PROPERTIES
      IMPORTED_LOCATION "${CHROMAPRINT_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${CHROMAPRINT_INCLUDE_DIR}"
      INTERFACE_COMPILE_DEFINITIONS "${CHROMAPRINT_COMPILE_DEFINITIONS}"
      INTERFACE_COMPILE_OPTIONS "${CHROMAPRINT_CFLAGS}"
    )
  endif()
endif()
