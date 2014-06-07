include_directories(
  ../core/config
  ../core/utils
  ../core/tags
  ../core/model
  ../core/import
  ../gui/forms
  ../gui/dialogs
)

if (BUILD_CLI_APP)
  add_subdirectory(cli)
endif (BUILD_CLI_APP)

if (BUILD_QT_APP)
  add_subdirectory(qt)
endif (BUILD_QT_APP)

if (BUILD_QML_APP)
  add_subdirectory(qml)
endif (BUILD_QML_APP)

if (BUILD_KDE_APP)
  set(_cmakeInstallRpath "${CMAKE_INSTALL_RPATH}")
  find_package(KDE4 REQUIRED)
  include (KDE4Defaults)

  if (BUILD_SHARED_LIBS)
    # FindKDE4Internal.cmake will overwrite our RPATH if LIB_INSTALL_DIR (which
    # defaults to ${CMAKE_INSTALL_PREFIX}/lib) is not an implicit link directory.
    # In practice this means that most values for CMAKE_INSTALL_PREFIX other
    # than /usr will cause the RPATH to be overwritten and the kid3 executable
    # will not find its libraries. This happens also for the default value
    # /usr/local for CMAKE_INSTALL_PREFIX.
    # The RPATH is set again here to reverse this.
    set(_prefixLibdir "${CMAKE_INSTALL_PREFIX}/${WITH_LIBDIR}")
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${_prefixLibdir}" _isSystemDir)
    if ("${_isSystemDir}" STREQUAL "-1")
      list(APPEND _cmakeInstallRpath ${CMAKE_INSTALL_RPATH})
      list(REMOVE_DUPLICATES _cmakeInstallRpath)
      set(CMAKE_INSTALL_RPATH "${_cmakeInstallRpath}")
    endif ("${_isSystemDir}" STREQUAL "-1")
  endif (BUILD_SHARED_LIBS)

  add_subdirectory(kde)
  kde4_install_icons(${ICON_INSTALL_DIR})
endif (BUILD_KDE_APP)
