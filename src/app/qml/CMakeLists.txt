set(qml_SRCS
  mainqml.cpp
)

add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
add_executable(kid3-qml MACOSX_BUNDLE WIN32 ${qml_SRCS})

target_link_libraries(kid3-qml kid3-gui -lstdc++)

install(TARGETS kid3-qml
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${WITH_BINDIR})
