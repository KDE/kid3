### Check for readline
set(READLINE_INCLUDE_DIR)
set(READLINE_LIBRARIES)
if (WITH_READLINE)
  find_path(READLINE_INCLUDE_DIR readline/readline.h)
  find_library(READLINE_LIBRARY readline)
  if (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
    # Check whether it needs to be linked against termcap
    set(_CMAKE_REQUIRED_INCLUDES_TMP "${CMAKE_REQUIRED_INCLUDES}")
    set(_CMAKE_REQUIRED_LIBRARIES_TMP "${CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_INCLUDES "${READLINE_INCLUDE_DIR}")
    set(CMAKE_REQUIRED_LIBRARIES "${READLINE_LIBRARY}")
    CHECK_CXX_SOURCE_COMPILES("#include<cstdio>\n#include <readline/readline.h>\nint main() {\n  readline(\"\");\n  return 0;\n}\n" READLINE_STANDALONE_FOUND)
    if (READLINE_STANDALONE_FOUND)
      set(READLINE_LIBRARIES ${READLINE_LIBRARY})
    else (READLINE_STANDALONE_FOUND)
      # Check whether it works with termcap
      find_library(TERMCAP_LIBRARY termcap)
      if (TERMCAP_LIBRARY)
        set(CMAKE_REQUIRED_LIBRARIES "${READLINE_LIBRARY};${TERMCAP_LIBRARY}")
        CHECK_CXX_SOURCE_COMPILES("#include<cstdio>\n#include <readline/readline.h>\nint main() {\n  readline(\"\");\n  return 0;\n}\n" READLINE_TERMCAP_FOUND)
        if (READLINE_TERMCAP_FOUND)
          set(READLINE_LIBRARIES "${READLINE_LIBRARY};${TERMCAP_LIBRARY}")
        endif (READLINE_TERMCAP_FOUND)
      endif (TERMCAP_LIBRARY)
    endif (READLINE_STANDALONE_FOUND)
    set(CMAKE_REQUIRED_INCLUDES "${_CMAKE_REQUIRED_INCLUDES_TMP}")
    set(CMAKE_REQUIRED_LIBRARIES "${_CMAKE_REQUIRED_LIBRARIES_TMP}")
  endif (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
  if (READLINE_LIBRARIES)
    message(STATUS "readline found: ${READLINE_LIBRARIES}")
    set(HAVE_READLINE 1)
  else (READLINE_LIBRARIES)
    message(FATAL_ERROR "Could not find readline")
  endif (READLINE_LIBRARIES)
endif (WITH_READLINE)
configure_file(cliconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/cliconfig.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${READLINE_INCLUDE_DIR})

set(cli_SRCS
  maincli.cpp
  abstractcli.cpp
  kid3cli.cpp
  clicommand.cpp
  standardinputreader.cpp
)
if (HAVE_READLINE)
  set(cli_SRCS ${cli_SRCS} readlinecompleter.cpp)
endif (HAVE_READLINE)

set(cli_MOC_HDRS
  abstractcli.h
  kid3cli.h
  clicommand.h
  standardinputreader.h
)
qt4_wrap_cpp(cli_GEN_MOC_SRCS ${cli_MOC_HDRS})
set(cli_SRCS ${cli_SRCS} ${cli_GEN_MOC_SRCS})

add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
add_executable(kid3-cli ${cli_SRCS} ${cli_GEN_RCS})

target_link_libraries(kid3-cli kid3-core ${READLINE_LIBRARIES} -lstdc++)

if (APPLE)
  set(cli_BINDIR ${WITH_LIBDIR})
else (APPLE)
  set(cli_BINDIR ${WITH_BINDIR})
endif (APPLE)

install(TARGETS kid3-cli
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${cli_BINDIR})
