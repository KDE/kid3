set(PLUGIN_LIBRARIES)
add_subdirectory(plugins)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(app)

if(WITH_QML)
  add_subdirectory(qml)
endif()

if(BUILD_TESTING AND BUILD_TEST_APP)
  add_subdirectory(test)
endif()
