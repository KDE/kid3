find_package(PkgConfig)
set(WITH_GSTREAMER_VERSION "" CACHE STRING
  "GStreamer version to use or list of versions to check")
if(WITH_GSTREAMER_VERSION)
  set(_checkedGstVersions ${WITH_GSTREAMER_VERSION})
else()
  set(_checkedGstVersions "1.0;0.10")
endif()
foreach(_gstVersion ${_checkedGstVersions})
  pkg_check_modules(GSTREAMER gstreamer-${_gstVersion})
  if(GSTREAMER_FOUND)
    find_library(GSTREAMER_LIBRARY NAMES gstreamer-${_gstVersion})
    if(NOT WITH_GSTREAMER_VERSION)
      message(STATUS "GStreamer ${_gstVersion} selected, "
        "use WITH_GSTREAMER_VERSION for another version.")
    endif()
    break()
  endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GStreamer
                                  REQUIRED_VARS GSTREAMER_LIBRARY GSTREAMER_INCLUDE_DIRS
                                  VERSION_VAR GSTREAMER_VERSION)

if(GSTREAMER_FOUND)
  if(NOT TARGET GStreamer::GStreamer)
    add_library(GStreamer::GStreamer UNKNOWN IMPORTED)
    set_target_properties(GStreamer::GStreamer PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${GSTREAMER_INCLUDE_DIRS}"
      IMPORTED_LOCATION "${GSTREAMER_LIBRARY}"
      INTERFACE_COMPILE_OPTIONS "${GSTREAMER_CFLAGS_OTHER}"
    )
  endif()
endif()
