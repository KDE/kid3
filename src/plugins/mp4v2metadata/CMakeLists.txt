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

  configure_file(mp4v2config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/mp4v2config.h)

  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${MP4V2_INCLUDE_DIR} ../../core/tags)

  if (WIN32)
    add_definitions(-DMP4V2_USE_STATIC_LIB)
  endif (WIN32)

  set(plugin_SRCS
    mp4v2metadataplugin.cpp
    m4afile.cpp
  )

  set(plugin_MOC_HDRS
    mp4v2metadataplugin.h
  )

  set(plugin_NAME Mp4v2Metadata)

  if (WITH_GCC_PCH)
    add_definitions(${GCC_PCH_COMPILE_FLAGS})
  endif (WITH_GCC_PCH)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  qt5_wrap_cpp(plugin_GEN_MOC_SRCS ${plugin_MOC_HDRS})

  add_library(${plugin_TARGET} ${plugin_SRCS} ${plugin_GEN_MOC_SRCS})
  target_link_libraries(${plugin_TARGET} kid3-core ${BASIC_LIBRARIES} ${MP4V2_LIBRARIES})

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif(WITH_MP4V2)
