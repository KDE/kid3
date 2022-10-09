set(_languages ca ca@valencia cs da de el en en_GB es et eu fi fr is it ko nl nn pl pt pt_BR ru sl sr sr@ijekavian sr@ijekavianlatin sr@latin sv tr uk zh_CN zh_TW)

# Use -DDOWNLOAD_POS=ON to download the translations directly from SVN.
if(DOWNLOAD_POS STREQUAL "REMOVE")
  message(STATUS "Removing po")
  file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/po")
elseif(DOWNLOAD_POS)
  execute_process(COMMAND ${CMAKE_COMMAND}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/download-pos.cmake" "${_languages}"
    RESULT_VARIABLE result)
  if(NOT "${result}" STREQUAL "0")
    message(STATUS "Download failed, removing po")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/po")
  endif()
endif()

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
  set(_poDir ${CMAKE_CURRENT_SOURCE_DIR}/po)
  message(WARNING "Using custom translations from ${_poDir}")
else()
  set(_poDir ${CMAKE_SOURCE_DIR}/poqm)
endif()

# Using ecm_install_po_files_as_qm() from ECMPoQmTools does not work
# because the generated ts files do not have context information,
# just one single context with an empty name.

set(_poFiles)
set(_tsFiles)

foreach(_lang ${_languages})
  set(_poFiles ${_poFiles} ${_poDir}/${_lang}/kid3_qt.po)
  set(_tsFiles ${_tsFiles} ${CMAKE_CURRENT_BINARY_DIR}/kid3_${_lang}.ts)
endforeach()

if(APPLE OR WIN32 OR ANDROID OR LINUX_SELF_CONTAINED)
  if(NOT QT_TRANSLATIONS_DIR)
    if(TARGET Qt${QT_VERSION_MAJOR}::qmake)
      get_target_property(QT_TRANSLATIONS_DIR Qt${QT_VERSION_MAJOR}::qmake LOCATION)
    endif()
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
       ${QT_LUPDATE_EXECUTABLE} ${_poDir}
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
  qt_add_translation(_qmFiles ${_tsFiles} ${_qtTsFiles})
  add_custom_target(translations "ALL" DEPENDS ${_qmFiles} ${_qtCopiedQmFiles})
  set(_installQmFiles ${_qmFiles} ${_qtQmFiles})
else()
  qt_add_translation(_qmFiles ${_tsFiles})
  add_custom_target(translations "ALL" DEPENDS ${_qmFiles})
  set(_installQmFiles ${_qmFiles})
endif()
if(NOT HAVE_TRANSLATIONSDIR_IN_QRC)
  install(FILES ${_installQmFiles} DESTINATION ${WITH_TRANSLATIONSDIR})
endif()
