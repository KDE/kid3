if (NOT WITH_KDE)

# Create HTML handbook from DocBook file in language directory
macro (kid3_create_handbook _docbook)
  get_filename_component(_input ${_docbook} ABSOLUTE)
  get_filename_component(_lang ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(_output ${CMAKE_CURRENT_BINARY_DIR}/kid3_${_lang}.html)
  set(_fixdocbook_pl ${CMAKE_CURRENT_SOURCE_DIR}/../fixdocbook.pl)
  set(_fixhtml_pl ${CMAKE_CURRENT_SOURCE_DIR}/../fixhtml.pl)
  set(_docbook_xsl ${DOCBOOK_XSL_DIR}/html/docbook.xsl)
  if (WIN32)
    # The expressions in pipes do not work for Windows,
    # so temporary files are used.
    set(_tmp_xml ${CMAKE_CURRENT_BINARY_DIR}/tmp.xml)
    set(_tmp_html ${CMAKE_CURRENT_BINARY_DIR}/tmp.html)
    add_custom_command(OUTPUT ${_tmp_xml}
      COMMAND ${PERL_EXECUTABLE} ${_fixdocbook_pl} <${_input} >${_tmp_xml}
      DEPENDS ${_input}
    )
    add_custom_command(OUTPUT ${_tmp_html}
      COMMAND ${XSLTPROC} --novalid --nonet ${_docbook_xsl} ${_tmp_xml} >${_tmp_html}
      DEPENDS ${_tmp_xml}
    )
    add_custom_command(OUTPUT ${_output}
      COMMAND ${PERL_EXECUTABLE} -n ${_fixhtml_pl} <${_tmp_html} >${_output}
      DEPENDS ${_tmp_html}
    )
  else (WIN32)
    add_custom_command(OUTPUT ${_output}
      COMMAND 
        ${PERL_EXECUTABLE} ${_fixdocbook_pl} <${_input} |
        ${XSLTPROC} --novalid --nonet ${_docbook_xsl} - |
        ${PERL_EXECUTABLE} -n ${_fixhtml_pl} >${_output}
      DEPENDS ${_input}
    )
  endif (WIN32)
  add_custom_target("${_lang}-handbook" ALL DEPENDS ${_output})
  install(FILES ${_output} DESTINATION ${WITH_DOCDIR})
endmacro (kid3_create_handbook)

endif (NOT WITH_KDE)

add_subdirectory(en)
add_subdirectory(de)
