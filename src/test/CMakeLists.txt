qt_wrap_cpp(test_GEN_MOC_SRCS
  testserverimporterbase.h
  testmusicbrainzreleaseimporter.h
  testmusicbrainzreleaseimportparser.h
  testdiscogsimporter.h
  testamazonimporter.h
  TARGET kid3-test
)
add_executable(kid3-test
  dummysettings.cpp
  testutils.cpp
  testserverimporterbase.cpp
  testmusicbrainzreleaseimporter.cpp
  testmusicbrainzreleaseimportparser.cpp
  testdiscogsimporter.cpp
  testamazonimporter.cpp
  maintest.cpp
  ${test_GEN_MOC_SRCS}
)
target_link_libraries(kid3-test kid3-core Qt${QT_VERSION_MAJOR}::Test)
if(NOT MSVC)
  target_link_libraries(kid3-test -lstdc++)
endif()
