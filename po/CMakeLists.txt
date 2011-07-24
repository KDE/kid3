if(WITH_KDE)

# FindGettext.cmake, but without the sort option (-s) for msgmerge

FIND_PROGRAM(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

IF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   FILE(GLOB _poFiles *.po)
   SET(_potFile kid3.pot)
   SET(_gmoFiles)
   GET_FILENAME_COMPONENT(_potBasename ${_potFile} NAME_WE)
   GET_FILENAME_COMPONENT(_absPotFile ${_potFile} ABSOLUTE)

   FOREACH (_currentPoFile ${_poFiles})
      GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_abs_PATH ${_absFile} PATH)
      GET_FILENAME_COMPONENT(_lang ${_absFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_gmoFile}
         COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none ${_absFile} ${_absPotFile}
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_absFile}
         DEPENDS ${_absPotFile} ${_absFile}
      )

      INSTALL(FILES ${_gmoFile} DESTINATION share/locale/${_lang}/LC_MESSAGES RENAME ${_potBasename}.mo)
      SET(_gmoFiles ${_gmoFiles} ${_gmoFile})

   ENDFOREACH (_currentPoFile )

   ADD_CUSTOM_TARGET(translations "ALL" DEPENDS ${_gmoFiles})
ELSE (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   MESSAGE(FATAL_ERROR "GetText not found")
ENDIF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )

else(WITH_KDE)

  # Generate .ts files from the .po files and then convert them to .qm files.
  # Also copy or convert the Qt language files from the Qt installation.
  file(GLOB _poFiles *.po)
  set(_tsFiles)
  set(_qtQmFiles)
  set(_qtTsFiles)

  foreach (_currentPoFile ${_poFiles})
    get_filename_component(_lang ${_currentPoFile} NAME_WE)
    set(_tsFiles ${_tsFiles} ${CMAKE_CURRENT_BINARY_DIR}/kid3_${_lang}.ts)
    if (EXISTS ${QT_TRANSLATIONS_DIR}/qt_${_lang}.qm)
      set(_qtQmFiles ${_qtQmFiles} ${QT_TRANSLATIONS_DIR}/qt_${_lang}.qm)
    elseif (EXISTS ${QT_TRANSLATIONS_DIR}/qt_${_lang}.ts)
      set(_qtTsFiles ${_qtTsFiles} ${QT_TRANSLATIONS_DIR}/qt_${_lang}.ts)
    endif (EXISTS ${QT_TRANSLATIONS_DIR}/qt_${_lang}.qm)
  endforeach (_currentPoFile)

  add_custom_command(
     OUTPUT ${_tsFiles}
     COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/po2ts.pl ${QT_LUPDATE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../kid3
     DEPENDS ${_poFiles}
  )
  qt4_add_translation(_qmFiles ${_tsFiles} ${_qtTsFiles})
  add_custom_target(translations "ALL" DEPENDS ${_qmFiles})
  install(FILES ${_qmFiles} ${_qtQmFiles} DESTINATION ${WITH_TRANSLATIONSDIR})

endif(WITH_KDE)
