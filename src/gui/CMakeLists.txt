include_directories(
  ../core/utils ../core/config ../core/tags ../core/export ../core/import ../core/model
  widgets dialogs
)

set(gui_SRCS)
set(gui_MOC_HDRS)
foreach (_subdir widgets dialogs forms)
  include(${_subdir}/Sources.cmake)
  set(gui_SRCS ${gui_SRCS} ${${_subdir}_SRCS})
  set(gui_MOC_HDRS ${gui_MOC_HDRS} ${${_subdir}_MOC_HDRS})
endforeach()

if (WITH_GCC_PCH)
  ADD_KID3_PCH_RULE(gui_SRCS)
endif (WITH_GCC_PCH)

qt4_wrap_cpp(gui_GEN_MOC_SRCS ${gui_MOC_HDRS})

add_library(kid3-gui ${gui_SRCS} ${gui_GEN_MOC_SRCS})
target_link_libraries(kid3-gui kid3-core)

if (BUILD_SHARED_LIBS)
  install(TARGETS kid3-gui
    LIBRARY DESTINATION ${WITH_LIBDIR}
    ARCHIVE DESTINATION ${WITH_LIBDIR}
    RUNTIME DESTINATION ${WITH_BINDIR})
endif (BUILD_SHARED_LIBS)

include_directories(forms)
add_subdirectory(app)