include_directories(
  ../core/utils
  ../core/model
  ../core/tags
  ../core/import
)

set(test_SRCS
testutils.cpp
testserverimporterbase.cpp
testmusicbrainzreleaseimporter.cpp
testmusicbrainzreleaseimportparser.cpp
main.cpp
)

set(test_MOC_HDRS
testserverimporterbase.h
testmusicbrainzreleaseimporter.h
testmusicbrainzreleaseimportparser.h
)

qt4_wrap_cpp(test_GEN_MOC_SRCS ${test_MOC_HDRS})
add_definitions(${QT_DEFINITIONS})
add_executable(kid3-test ${test_SRCS} ${test_GEN_MOC_SRCS})
target_link_libraries(kid3-test kid3-core ${QT_QTTEST_LIBRARY})
