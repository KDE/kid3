if (WITH_GCC_PCH)
  set(_headerFilename ${CMAKE_BINARY_DIR}/allsys.h)
  set(_gchFilename "${_headerFilename}.gch")

  if (NOT EXISTS ${_headerFilename})
    message(STATUS "Generating ${_headerFilename}")

    if (BUILD_KDE_APP)
      set(_includeRegex "#include <[Qkcp]")
    else (BUILD_KDE_APP)
      set(_includeRegex "#include <[Qcp]")
    endif (BUILD_KDE_APP)

    file(GLOB_RECURSE _srcList "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
                       "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
                       "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
                       "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    set(_includes)
    foreach (_src ${_srcList})
      file(STRINGS ${_src} _inc REGEX ${_includeRegex})
      if (NOT "${_inc}" MATCHES "#include (.config\\.h.|.QAudio.*|.QMedia.*|.Qt?Plugin.*|.Qt?Test.*)")
        list(APPEND _includes ${_inc})
      endif()
    endforeach (_src ${_srcList})
    list(SORT _includes)
    list(REMOVE_DUPLICATES _includes)
    set(_txt "/* automatically generated */\n"
             "#ifndef ALLSYS_H\n"
             "#define ALLSYS_H\n"
             "#ifdef __cplusplus\n")
    foreach(_inc ${_includes})
      set(_txt "${_txt}${_inc}\n")
    endforeach(_inc _includes)
    set(_txt "${_txt}#endif\n#endif\n")
    file(WRITE ${_headerFilename} ${_txt})

    message(STATUS "Generating ${_gchFilename}")

    file(REMOVE ${_gchFilename})
    set(_additionalFlags "-DQT_CORE_LIB -DQT_DBUS_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_XML_LIB -Wundef -Wcast-align -Wall -W -Wpointer-arith  -fno-check-new -fno-common -Woverloaded-virtual -D_REENTRANT -DQT_NO_EXCEPTIONS -DQT_NO_STL -fvisibility=hidden -fvisibility-inlines-hidden -DQT_ASCII_CAST_WARNINGS -DQT_NO_CAST_TO_ASCII")
    if (CMAKE_BUILD_TYPE MATCHES Debug)
      set(_additionalFlags "${_additionalFlags} -g -DQT_DEBUG")
    else (CMAKE_BUILD_TYPE MATCHES Debug)
      set(_additionalFlags "${_additionalFlags} -O3 -DQT_NO_DEBUG -DNDEBUG")
    endif (CMAKE_BUILD_TYPE MATCHES Debug)
    list(APPEND _args -c ${_headerFilename} -o ${_gchFilename} ${_additionalFlags})
    get_directory_property(_dirinc INCLUDE_DIRECTORIES)
    foreach (_inc ${_dirinc})
      list(APPEND _args "-I" ${_inc})
    endforeach(_inc ${_dirinc})
    separate_arguments(_args)
    execute_process(
         COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_args})
  endif (NOT EXISTS ${_headerFilename})

  set(GCC_PCH_COMPILE_FLAGS "-Winvalid-pch -include ${_headerFilename}")
endif (WITH_GCC_PCH)

set(PLUGIN_LIBRARIES)
add_subdirectory(plugins)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(app)

if (BUILD_TEST_APP)
  add_subdirectory(test)
endif (BUILD_TEST_APP)
