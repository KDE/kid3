include_directories(
  ../core/utils ../core/config ../core/tags ../core/export ../core/import ../core/model
  widgets dialogs forms
)

set(gui_SRCS)
set(gui_MOC_HDRS)
foreach (_subdir widgets dialogs forms)
  include(${_subdir}/Sources.cmake)
  set(gui_SRCS ${gui_SRCS} ${${_subdir}_SRCS})
  set(gui_MOC_HDRS ${gui_MOC_HDRS} ${${_subdir}_MOC_HDRS})
endforeach()

add_definitions(-DKID3_BUILD_GUI_LIB)
if (NOT BUILD_SHARED_LIBS)
  add_definitions(${QT_EXECUTABLE_COMPILE_FLAGS})
endif (NOT BUILD_SHARED_LIBS)
if (WITH_GCC_PCH)
  add_definitions(${GCC_PCH_COMPILE_FLAGS})
endif (WITH_GCC_PCH)

qt5_wrap_cpp(gui_GEN_MOC_SRCS ${gui_MOC_HDRS})

add_library(kid3-gui ${gui_SRCS} ${gui_GEN_MOC_SRCS})
set_target_properties(kid3-gui PROPERTIES VERSION ${KID3_VERSION})
target_link_libraries(kid3-gui kid3-core)

if (BUILD_SHARED_LIBS)
  install(TARGETS kid3-gui
    LIBRARY DESTINATION ${WITH_LIBDIR}
    RUNTIME DESTINATION ${WITH_BINDIR})
endif (BUILD_SHARED_LIBS)
