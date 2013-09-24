### Check for readline
set(READLINE_LIBRARIES)
if (WITH_READLINE)
  find_library(READLINE_LIBRARY readline)
  if (READLINE_LIBRARY)
    set(READLINE_LIBRARIES ${READLINE_LIBRARY})
    message(STATUS "readline found: ${READLINE_LIBRARIES}")
    set(HAVE_READLINE 1)
  else (READLINE_LIBRARY)
    message(FATAL_ERROR "Could not find readline")
  endif (READLINE_LIBRARY)
endif (WITH_READLINE)
configure_file(cliconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/cliconfig.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

target_link_libraries(kid3-cli kid3-core ${READLINE_LIBRARIES})

if (APPLE)
  set(cli_BINDIR ${WITH_LIBDIR})
else (APPLE)
  set(cli_BINDIR ${WITH_BINDIR})
endif (APPLE)

install(TARGETS kid3-cli
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${cli_BINDIR})
