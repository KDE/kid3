include_directories(utils config tags formats export import model)

set(core_SRCS)
set(core_MOC_HDRS)
foreach (_subdir utils config tags formats export import model)
  include(${_subdir}/Sources.cmake)
  set(core_SRCS ${core_SRCS} ${${_subdir}_SRCS})
  set(core_MOC_HDRS ${core_MOC_HDRS} ${${_subdir}_MOC_HDRS})
endforeach()

add_definitions(-DKID3_BUILD_CORE_LIB)
if (WIN32 AND HAVE_CHROMAPRINT)
  add_definitions(-DCHROMAPRINT_NODLL)
endif (WIN32 AND HAVE_CHROMAPRINT)

if (WITH_GCC_PCH)
  ADD_KID3_PCH_RULE(core_SRCS)
endif (WITH_GCC_PCH)

qt4_wrap_cpp(core_GEN_MOC_SRCS ${core_MOC_HDRS})

add_library(kid3-core ${core_SRCS} ${core_GEN_MOC_SRCS})
target_link_libraries(kid3-core
  ${BASIC_LIBRARIES} ${ID3LIB_LIBRARIES} ${TAGLIB_LIBRARIES} ${VORBIS_LIBRARIES}
  ${FLAC_LIBRARIES} ${CHROMAPRINT_LIBS} ${MP4V2_LIBRARIES}
)

if (BUILD_SHARED_LIBS)
  install(TARGETS kid3-core
    LIBRARY DESTINATION ${WITH_LIBDIR}
    ARCHIVE DESTINATION ${WITH_LIBDIR}
    RUNTIME DESTINATION ${WITH_BINDIR})
endif (BUILD_SHARED_LIBS)

if (WITH_KDE)
  install(FILES model/net.sourceforge.Kid3.xml
          DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
endif (WITH_KDE)
