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
