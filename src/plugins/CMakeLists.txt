project(kid3_plugins)

# Install a library _target as a plugin with _name.
macro (INSTALL_KID3_PLUGIN _target _name)
  if (BUILD_SHARED_LIBS)
    install(TARGETS ${_target}
      LIBRARY DESTINATION ${WITH_PLUGINSDIR}
      RUNTIME DESTINATION ${WITH_PLUGINSDIR})
    set_target_properties(
      ${_target} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}
      LIBRARY_OUTPUT_DIRECTORY ${kid3_plugins_BINARY_DIR}
    )
  else (BUILD_SHARED_LIBS)
    set_property(
      TARGET ${_target}
      APPEND PROPERTY COMPILE_DEFINITIONS QT_STATICPLUGIN
    )
    set(PLUGIN_LIBRARIES ${PLUGIN_LIBRARIES} ${_target} PARENT_SCOPE)
    set(PLUGIN_NAMES ${PLUGIN_NAMES} ${_name}Plugin PARENT_SCOPE)
  endif (BUILD_SHARED_LIBS)
endmacro (INSTALL_KID3_PLUGIN _target _name)

add_definitions(-DKID3_BUILD_PLUGIN_LIB)
if (NOT BUILD_SHARED_LIBS)
  add_definitions(${QT_EXECUTABLE_COMPILE_FLAGS})
endif (NOT BUILD_SHARED_LIBS)

include_directories(../core/utils ../core/model ../core/tags ../core/config ../core/import)

add_subdirectory(amazonimport)
add_subdirectory(discogsimport)
add_subdirectory(freedbimport)
add_subdirectory(musicbrainzimport)
add_subdirectory(acoustidimport)
add_subdirectory(id3libmetadata)
add_subdirectory(taglibmetadata)
add_subdirectory(oggflacmetadata)
add_subdirectory(mp4v2metadata)
if (WITH_QML)
  add_subdirectory(qmlcommand)
  add_subdirectory(kid3qml)
endif (WITH_QML)

set(PLUGIN_LIBRARIES ${PLUGIN_LIBRARIES} PARENT_SCOPE)
set(CFG_IMPORT_PLUGIN_CALLS)
foreach(_pluginName ${PLUGIN_NAMES})
  set(CFG_IMPORT_PLUGIN_CALLS "${CFG_IMPORT_PLUGIN_CALLS}Q_IMPORT_PLUGIN(${_pluginName})\n")
endforeach()
configure_file(importplugins.h.cmake ${kid3_BINARY_DIR}/importplugins.h)
