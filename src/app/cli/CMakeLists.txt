### Check for readline
set(READLINE_INCLUDE_DIR)
set(READLINE_LIBRARIES)
if(WITH_READLINE)
  find_path(READLINE_INCLUDE_DIR readline/readline.h)
  find_library(READLINE_LIBRARY readline)
  if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
    # Check whether it needs to be linked against termcap
    set(_CMAKE_REQUIRED_INCLUDES_TMP "${CMAKE_REQUIRED_INCLUDES}")
    set(_CMAKE_REQUIRED_LIBRARIES_TMP "${CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_INCLUDES "${READLINE_INCLUDE_DIR}")
    set(CMAKE_REQUIRED_LIBRARIES "${READLINE_LIBRARY}")
    CHECK_CXX_SOURCE_COMPILES("#include<cstdio>\n#include <readline/readline.h>\nint main() {\n  readline(\"\");\n  return 0;\n}\n" READLINE_STANDALONE_FOUND)
    if(READLINE_STANDALONE_FOUND)
      set(READLINE_LIBRARIES ${READLINE_LIBRARY})
    else()
      # Check whether it works with termcap
      find_library(TERMCAP_LIBRARY termcap)
      if(TERMCAP_LIBRARY)
        set(CMAKE_REQUIRED_LIBRARIES "${READLINE_LIBRARY};${TERMCAP_LIBRARY}")
        CHECK_CXX_SOURCE_COMPILES("#include<cstdio>\n#include <readline/readline.h>\nint main() {\n  readline(\"\");\n  return 0;\n}\n" READLINE_TERMCAP_FOUND)
        if(READLINE_TERMCAP_FOUND)
          set(READLINE_LIBRARIES "${READLINE_LIBRARY};${TERMCAP_LIBRARY}")
        endif()
      endif()
    endif()
    set(CMAKE_REQUIRED_INCLUDES "${_CMAKE_REQUIRED_INCLUDES_TMP}")
    set(CMAKE_REQUIRED_LIBRARIES "${_CMAKE_REQUIRED_LIBRARIES_TMP}")
  endif()
  if(READLINE_LIBRARIES)
    message(STATUS "readline found: ${READLINE_LIBRARIES}")
    set(HAVE_READLINE 1)
  else()
    message(FATAL_ERROR "Could not find readline")
  endif()
endif()
configure_file(cliconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/cliconfig.h)

add_executable(kid3-cli
  maincli.cpp
  abstractcli.cpp
  kid3cli.cpp
  clicommand.cpp
  standardiohandler.cpp
  abstractcliformatter.cpp
  textcliformatter.cpp
  jsoncliformatter.cpp
)

if(HAVE_READLINE)
  target_sources(kid3-cli PRIVATE readlinecompleter.cpp)
endif()

qt5_wrap_cpp(cli_GEN_MOC_SRCS
  abstractcli.h
  kid3cli.h
  clicommand.h
  standardiohandler.h
  textcliformatter.h
  jsoncliformatter.h
  TARGET kid3-cli
)
target_sources(kid3-cli PRIVATE ${cli_GEN_MOC_SRCS})

target_include_directories(kid3-cli PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${READLINE_INCLUDE_DIR})

target_link_libraries(kid3-cli kid3-core ${READLINE_LIBRARIES} -lstdc++)

if(APPLE)
  set(cli_BINDIR ${WITH_LIBDIR})
else()
  set(cli_BINDIR ${WITH_BINDIR})
endif()

install(TARGETS kid3-cli
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${cli_BINDIR})

if(LINUX_SELF_CONTAINED)
  set_target_properties(kid3-cli PROPERTIES INSTALL_RPATH "\$ORIGIN")
endif()
