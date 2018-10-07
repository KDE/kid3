# Generate .ts files from the .po files and then convert them to .qm files.
file(GLOB _poFiles *.po)
set(_tsFiles)

foreach(_currentPoFile ${_poFiles})
  get_filename_component(_lang ${_currentPoFile} NAME_WE)
  set(_tsFiles ${_tsFiles} ${CMAKE_CURRENT_BINARY_DIR}/kid3_${_lang}.ts)
endforeach()

if(APPLE OR WIN32 OR ANDROID)
  if(NOT QT_TRANSLATIONS_DIR)
    get_target_property(QT_TRANSLATIONS_DIR Qt5::qmake LOCATION)
    get_filename_component(QT_TRANSLATIONS_DIR ${QT_TRANSLATIONS_DIR} PATH)
    get_filename_component(QT_TRANSLATIONS_DIR ${QT_TRANSLATIONS_DIR} PATH)
    set(QT_TRANSLATIONS_DIR "${QT_TRANSLATIONS_DIR}/translations")
  endif()

  # Also copy or convert the Qt language files from the Qt installation.
  set(_qtQmFiles)
  set(_qtTsFiles)

  set(_qttranslation_prefix "${QT_TRANSLATIONS_DIR}/qtbase_")
  foreach(_currentPoFile ${_poFiles})
    get_filename_component(_lang ${_currentPoFile} NAME_WE)
    if(EXISTS ${_qttranslation_prefix}${_lang}.qm)
      set(_qtQmFiles ${_qtQmFiles} ${_qttranslation_prefix}${_lang}.qm)
    elseif(EXISTS ${_qttranslation_prefix}${_lang}.ts)
      set(_qtTsFiles ${_qtTsFiles} ${_qttranslation_prefix}${_lang}.ts)
    endif()
  endforeach()
endif()

if(_tsFiles)
  add_custom_command(
     OUTPUT ${_tsFiles}
     COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/po2ts.py
       ${QT_LUPDATE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}
       ${CMAKE_CURRENT_SOURCE_DIR}/../src
     DEPENDS ${_poFiles}
  )
endif()
if(APPLE OR WIN32)
  qt5_add_translation(_qmFiles ${_tsFiles} ${_qtTsFiles})
  add_custom_target(translations "ALL" DEPENDS ${_qmFiles})
  set(_installQmFiles ${_qmFiles} ${_qtQmFiles})
else()
  qt5_add_translation(_qmFiles ${_tsFiles})
  add_custom_target(translations "ALL" DEPENDS ${_qmFiles})
  set(_installQmFiles ${_qmFiles})
endif()
if(NOT HAVE_TRANSLATIONSDIR_IN_QRC)
  install(FILES ${_installQmFiles} DESTINATION ${WITH_TRANSLATIONSDIR})
endif()
