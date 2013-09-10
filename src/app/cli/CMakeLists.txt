set(cli_SRCS
  maincli.cpp
  abstractcli.cpp
  kid3cli.cpp
  clicommand.cpp
  standardinputreader.cpp
)
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

target_link_libraries(kid3-cli kid3-core)

install(TARGETS kid3-cli
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${WITH_BINDIR})
