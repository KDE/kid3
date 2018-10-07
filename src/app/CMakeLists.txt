if(BUILD_CLI_APP)
  add_subdirectory(cli)
endif()

if(BUILD_QT_APP)
  add_subdirectory(qt)
endif()

if(BUILD_QML_APP)
  add_subdirectory(qml)
endif()

if(BUILD_KDE_APP)
  set(_cmakeInstallRpath "${CMAKE_INSTALL_RPATH}")
  cmake_minimum_required(VERSION 3.3)
  find_package(ECM REQUIRED NO_MODULE)
  set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
  include(ECMInstallIcons)

  include(KDEInstallDirs)
  include(KDECompilerSettings)
  include(KDECMakeSettings)
  include(FeatureSummary)

  find_package(KF5 REQUIRED COMPONENTS Config ConfigWidgets CoreAddons WidgetsAddons XmlGui)

  if(BUILD_SHARED_LIBS)
    # KDECMakeSettings.cmake will overwrite our RPATH if LIB_INSTALL_DIR is
    # not an implicit link directory. In practice this means that most values
    # for CMAKE_INSTALL_PREFIX other than /usr will cause the RPATH to be
    # overwritten and the kid3 executable will not find its libraries.
    # This happens also for the default value /usr/local for
    # CMAKE_INSTALL_PREFIX. The RPATH is set again here to reverse this.
    set(_prefixLibdir "${CMAKE_INSTALL_PREFIX}/${WITH_LIBDIR}")
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${_prefixLibdir}" _isSystemDir)
    if("${_isSystemDir}" STREQUAL "-1")
      list(APPEND _cmakeInstallRpath ${CMAKE_INSTALL_RPATH})
      list(REMOVE_DUPLICATES _cmakeInstallRpath)
      set(CMAKE_INSTALL_RPATH "${_cmakeInstallRpath}")
    endif()
  endif()

  kde_enable_exceptions()
  add_subdirectory(kde)
  ecm_install_icons(ICONS
    sc-apps-kid3.svgz
    128-apps-kid3.png
    16-apps-kid3.png
    32-apps-kid3.png
    48-apps-kid3.png
    64-apps-kid3.png
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
  )
endif()
