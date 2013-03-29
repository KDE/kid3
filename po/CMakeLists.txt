# Generate .ts files from the .po files and then convert them to .qm files.
file(GLOB _poFiles *.po)
set(_tsFiles)

foreach (_currentPoFile ${_poFiles})
  get_filename_component(_lang ${_currentPoFile} NAME_WE)
  set(_tsFiles ${_tsFiles} ${CMAKE_CURRENT_BINARY_DIR}/kid3_${_lang}.ts)
endforeach (_currentPoFile)

if (APPLE OR WIN32)
  # Also copy or convert the Qt language files from the Qt installation.
  set(_qtQmFiles)
  set(_qtTsFiles)

  foreach (_currentPoFile ${_poFiles})
    get_filename_component(_lang ${_currentPoFile} NAME_WE)
    if (EXISTS ${QT_TRANSLATIONS_DIR}/qt_${_lang}.qm)
      set(_qtQmFiles ${_qtQmFiles} ${QT_TRANSLATIONS_DIR}/qt_${_lang}.qm)
    elseif (EXISTS ${QT_TRANSLATIONS_DIR}/qt_${_lang}.ts)
      set(_qtTsFiles ${_qtTsFiles} ${QT_TRANSLATIONS_DIR}/qt_${_lang}.ts)
    endif (EXISTS ${QT_TRANSLATIONS_DIR}/qt_${_lang}.qm)
  endforeach (_currentPoFile)
endif (APPLE OR WIN32)

add_custom_command(
   OUTPUT ${_tsFiles}
   COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/po2ts.pl
     ${QT_LUPDATE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/../src
   DEPENDS ${_poFiles}
)
if (APPLE OR WIN32)
  qt4_add_translation(_qmFiles ${_tsFiles} ${_qtTsFiles})
  add_custom_target(translations "ALL" DEPENDS ${_qmFiles})
  install(FILES ${_qmFiles} ${_qtQmFiles} DESTINATION ${WITH_TRANSLATIONSDIR})
else (APPLE OR WIN32)
  qt4_add_translation(_qmFiles ${_tsFiles})
  add_custom_target(translations "ALL" DEPENDS ${_qmFiles})
  install(FILES ${_qmFiles} DESTINATION ${WITH_TRANSLATIONSDIR})
endif (APPLE OR WIN32)
