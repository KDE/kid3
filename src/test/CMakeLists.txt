include_directories(
  ${QT_QTTEST_INCLUDE_DIR}
  ../core/utils
  ../core/model
  ../core/tags
  ../core/import
  ../core/config
)

set(test_SRCS
dummysettings.cpp
testutils.cpp
testjsonparser.cpp
testserverimporterbase.cpp
testmusicbrainzreleaseimporter.cpp
testmusicbrainzreleaseimportparser.cpp
testdiscogsimporter.cpp
maintest.cpp
)

set(test_MOC_HDRS
testjsonparser.h
testserverimporterbase.h
testmusicbrainzreleaseimporter.h
testmusicbrainzreleaseimportparser.h
testdiscogsimporter.h
)

qt5_wrap_cpp(test_GEN_MOC_SRCS ${test_MOC_HDRS})
add_definitions(${QT_DEFINITIONS} ${QT_EXECUTABLE_COMPILE_FLAGS})
add_executable(kid3-test ${test_SRCS} ${test_GEN_MOC_SRCS})
target_link_libraries(kid3-test kid3-core ${QT_QTTEST_LIBRARY} -lstdc++)
