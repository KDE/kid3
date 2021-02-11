qt5_wrap_cpp(test_GEN_MOC_SRCS
  testserverimporterbase.h
  testmusicbrainzreleaseimporter.h
  testmusicbrainzreleaseimportparser.h
  testdiscogsimporter.h
  TARGET kid3-test
)
add_executable(kid3-test
  dummysettings.cpp
  testutils.cpp
  testserverimporterbase.cpp
  testmusicbrainzreleaseimporter.cpp
  testmusicbrainzreleaseimportparser.cpp
  testdiscogsimporter.cpp
  maintest.cpp
  ${test_GEN_MOC_SRCS}
)
target_link_libraries(kid3-test kid3-core Qt5::Test)
if(NOT MSVC)
  target_link_libraries(kid3-test -lstdc++)
endif()
