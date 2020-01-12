if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
  # Using ecm_install_po_files_as_qm() from ECMPoQmTools does not work
  # because the generated ts files do not have context information,
  # just one single context with an empty name.

  file(GLOB _languages RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/po" po/*)
  list(REMOVE_ITEM _languages kid3_qt.pot)
  set(_poFiles)
  set(_tsFiles)

  foreach(_lang ${_languages})
    set(_poFiles ${_poFiles} ${CMAKE_CURRENT_SOURCE_DIR}/po/${_lang}/kid3_qt.po)
    set(_tsFiles ${_tsFiles} ${CMAKE_CURRENT_BINARY_DIR}/kid3_${_lang}.ts)
  endforeach()

  if(APPLE OR WIN32 OR ANDROID OR LINUX_SELF_CONTAINED)
    if(NOT QT_TRANSLATIONS_DIR)
      get_target_property(QT_TRANSLATIONS_DIR Qt5::qmake LOCATION)
      get_filename_component(QT_TRANSLATIONS_DIR ${QT_TRANSLATIONS_DIR} PATH)
      get_filename_component(QT_TRANSLATIONS_DIR ${QT_TRANSLATIONS_DIR} PATH)
      set(QT_TRANSLATIONS_DIR "${QT_TRANSLATIONS_DIR}/translations")
    endif()

    # Also copy or convert the Qt language files from the Qt installation.
    set(_qtQmFiles)
    set(_qtTsFiles)
    set(_qtCopiedQmFiles)

    set(_qttranslation_prefix "${QT_TRANSLATIONS_DIR}/qtbase_")
    set(_qtCopiedQm_prefix "${CMAKE_CURRENT_BINARY_DIR}/qtbase_")
    foreach(_lang ${_languages})
      if(EXISTS ${_qttranslation_prefix}${_lang}.qm)
        set(_qtQmFiles ${_qtQmFiles} ${_qttranslation_prefix}${_lang}.qm)
        if(HAVE_TRANSLATIONSDIR_IN_QRC)
          set(_qtCopiedQmFiles ${_qtCopiedQmFiles} ${_qtCopiedQm_prefix}${_lang}.qm)
        endif()
      elseif(EXISTS ${_qttranslation_prefix}${_lang}.ts)
        set(_qtTsFiles ${_qtTsFiles} ${_qttranslation_prefix}${_lang}.ts)
      endif()
    endforeach()
  endif()

  if(_tsFiles)
    add_custom_command(
       OUTPUT ${_tsFiles}
       COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/po2ts.py
         ${QT_LUPDATE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/po
         ${CMAKE_CURRENT_SOURCE_DIR}/../src
       DEPENDS ${_poFiles}
    )
  endif()

  if(_qtCopiedQmFiles)
    add_custom_command(
      OUTPUT ${_qtCopiedQmFiles}
      COMMAND ${CMAKE_COMMAND} -E copy ${_qtQmFiles} ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${_qtQmFiles}
    )
  endif()

  if(APPLE OR WIN32 OR ANDROID OR LINUX_SELF_CONTAINED)
    qt5_add_translation(_qmFiles ${_tsFiles} ${_qtTsFiles})
    add_custom_target(translations "ALL" DEPENDS ${_qmFiles} ${_qtCopiedQmFiles})
    set(_installQmFiles ${_qmFiles} ${_qtQmFiles})
  else()
    qt5_add_translation(_qmFiles ${_tsFiles})
    add_custom_target(translations "ALL" DEPENDS ${_qmFiles})
    set(_installQmFiles ${_qmFiles})
  endif()
  if(NOT HAVE_TRANSLATIONSDIR_IN_QRC)
    install(FILES ${_installQmFiles} DESTINATION ${WITH_TRANSLATIONSDIR})
  endif()
endif()
