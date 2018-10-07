if(MP4V2_INCLUDE_DIR)
  set(MP4V2_FIND_QUIETLY TRUE)
endif()

find_path(MP4V2_INCLUDE_DIR mp4.h
 /usr/include/
 /usr/local/include/
)
find_path(MP4V2_MP4V2_INCLUDE_DIR mp4v2/mp4v2.h
 /usr/include/
 /usr/local/include/
)
find_library(MP4V2_LIBRARY NAMES mp4v2
 PATHS /usr/lib
       /usr/local/lib
)

if(MP4V2_MP4V2_INCLUDE_DIR)
  set(MP4V2_INCLUDE_DIR ${MP4V2_MP4V2_INCLUDE_DIR})
  set(HAVE_MP4V2_MP4V2_H 1)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mp4v2 REQUIRED_VARS MP4V2_LIBRARY MP4V2_INCLUDE_DIR)

if(MP4V2_FOUND)
  set(HAVE_MP4V2 1)

  if(WIN32)
    set(MP4V2_DEFINITIONS -DMP4V2_USE_STATIC_LIB)
  endif()

  set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${MP4V2_LIBRARY})
  set(_CMAKE_REQUIRED_DEFINITIONS_TMP ${CMAKE_REQUIRED_DEFINITIONS})
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${MP4V2_DEFINITIONS})
  set(_CMAKE_REQUIRED_INCLUDES_TMP ${CMAKE_REQUIRED_INCLUDES})
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${MP4V2_INCLUDE_DIR})
  if(MP4V2_MP4V2_INCLUDE_DIR)
    CHECK_CXX_SOURCE_COMPILES("#include <mp4v2/mp4v2.h>\nint main() {\n  MP4FileHandle hFile;\n  uint32_t index;\n  char* ppName;\n  uint8_t* ppValue;\n  uint32_t pValueSize;\n  MP4GetMetadataByIndex(hFile, index, &ppName, &ppValue, &pValueSize);\n  return 0;\n}\n" HAVE_MP4V2_MP4GETMETADATABYINDEX_CHARPP_ARG)
  else()
    CHECK_CXX_SOURCE_COMPILES("#include <mp4.h>\nint main() {\n  MP4FileHandle hFile;\n  u_int32_t index;\n  char* ppName;\n  u_int8_t* ppValue;\n  u_int32_t pValueSize;\n  MP4GetMetadataByIndex(hFile, index, &ppName, &ppValue, &pValueSize);\n  return 0;\n}\n" HAVE_MP4V2_MP4GETMETADATABYINDEX_CHARPP_ARG)
  endif()
  set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
  set(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_TMP})
  set(CMAKE_REQUIRED_INCLUDES ${_CMAKE_REQUIRED_INCLUDES_TMP})

  if(NOT TARGET Mp4v2::Mp4v2)
    add_library(Mp4v2::Mp4v2 UNKNOWN IMPORTED)
    string(REPLACE "-D" "" MP4V2_COMPILE_DEFINITIONS "${MP4V2_DEFINITIONS}")
    set_target_properties(Mp4v2::Mp4v2 PROPERTIES
      IMPORTED_LOCATION "${MP4V2_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS "${MP4V2_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${MP4V2_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(MP4V2_INCLUDE_DIR MP4V2_LIBRARY)
