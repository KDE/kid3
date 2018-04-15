if(WITH_ID3LIB)
  find_path(ID3LIB_INCLUDE_DIR id3/tag.h)
  find_library(ID3LIB_LIBRARIES NAMES id3 id3lib)
  if(ID3LIB_INCLUDE_DIR AND ID3LIB_LIBRARIES)
    message(STATUS "id3lib found: ${ID3LIB_LIBRARIES}")
    set(HAVE_ID3LIB 1)
    set(ID3LIB_LIBRARIES ${ID3LIB_LIBRARIES} ${ZLIB_LIBRARIES})
    if (NOT MSVC)
      set(ID3LIB_LIBRARIES ${ID3LIB_LIBRARIES} -lstdc++)
    endif (NOT MSVC)
    if (WIN32)
      set(ID3LIB_DEFINITIONS -DID3LIB_LINKOPTION=1)
      if (NOT CMAKE_CROSSCOMPILING)
        set(ID3LIB_LIBRARIES ${ID3LIB_LIBRARIES} -liconv)
        # libiconv-2.dll is at the same place where the compiler is, C:\msys64\mingw32\bin
        get_filename_component(_mingw_bin_dir ${CMAKE_C_COMPILER} DIRECTORY)
        install(FILES "${_mingw_bin_dir}/libiconv-2.dll"
                DESTINATION ${WITH_BINDIR} OPTIONAL)
      endif (NOT CMAKE_CROSSCOMPILING)
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

  configure_file(id3libconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/id3libconfig.h)

  add_definitions(${ID3LIB_DEFINITIONS})
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${ID3LIB_INCLUDE_DIR} ../../core/tags)

  set(plugin_SRCS
    id3libmetadataplugin.cpp
    mp3file.cpp
  )

  set(plugin_MOC_HDRS
    id3libmetadataplugin.h
  )

  set(plugin_NAME Id3libMetadata)

  if (WITH_GCC_PCH)
    add_definitions(${GCC_PCH_COMPILE_FLAGS})
  endif (WITH_GCC_PCH)

  string(TOLOWER ${plugin_NAME} plugin_TARGET)

  qt4_wrap_cpp(plugin_GEN_MOC_SRCS ${plugin_MOC_HDRS})

  add_library(${plugin_TARGET} ${plugin_SRCS} ${plugin_GEN_MOC_SRCS})
  target_link_libraries(${plugin_TARGET} kid3-core ${BASIC_LIBRARIES} ${ID3LIB_LIBRARIES})

  INSTALL_KID3_PLUGIN(${plugin_TARGET} ${plugin_NAME})
endif(WITH_ID3LIB)
