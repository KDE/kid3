set(_languages ca de it nl pt ru sv uk)

# Use -DDOWNLOAD_POS=ON to download the translations directly from SVN.
if(DOWNLOAD_POS STREQUAL "REMOVE")
  message(STATUS "Removing po, docs")
  file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/po")
  file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/docs")
elseif(DOWNLOAD_POS)
  execute_process(COMMAND ${CMAKE_COMMAND}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/download-pos.cmake" "${_languages}"
    RESULT_VARIABLE result)
  if(NOT "${result}" STREQUAL "0")
    message(STATUS "Download failed, removing po, docs")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/po")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/docs")
  endif()
endif()

# Create HTML handbook from DocBook file in language directory
macro(kid3_create_handbook _docbook _lang)
  get_filename_component(_input ${_docbook} ABSOLUTE)
  set(_output ${CMAKE_CURRENT_BINARY_DIR}/${_lang}/kid3_${_lang}.html)
  set(_fixdocbook_py ${CMAKE_CURRENT_SOURCE_DIR}/fixdocbook.py)
  set(_fixhtml_py ${CMAKE_CURRENT_SOURCE_DIR}/fixhtml.py)
  set(_docbook_xsl ${DOCBOOK_XSL_DIR}/xhtml/docbook.xsl)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_lang})
  if(WIN32)
    # The expressions in pipes do not work for Windows,
    # so temporary files are used.
    set(_tmp_xml ${CMAKE_CURRENT_BINARY_DIR}/${_lang}/tmp.xml)
    set(_tmp_html ${CMAKE_CURRENT_BINARY_DIR}/${_lang}/tmp.html)
    add_custom_command(OUTPUT ${_tmp_xml}
      COMMAND ${PYTHON_EXECUTABLE} ${_fixdocbook_py} ${_input} >${_tmp_xml}
      DEPENDS ${_input}
    )
    add_custom_command(OUTPUT ${_tmp_html}
      COMMAND ${XSLTPROC} --novalid --nonet ${_docbook_xsl} ${_tmp_xml} >${_tmp_html}
      DEPENDS ${_tmp_xml}
    )
    add_custom_command(OUTPUT ${_output}
      COMMAND ${PYTHON_EXECUTABLE} ${_fixhtml_py} <${_tmp_html} >${_output}
      DEPENDS ${_tmp_html}
    )
  else()
    add_custom_command(OUTPUT ${_output}
      COMMAND
        ${PYTHON_EXECUTABLE} ${_fixdocbook_py} ${_input} |
        ${XSLTPROC} --maxdepth 20000 --novalid --nonet ${_docbook_xsl} - |
        ${PYTHON_EXECUTABLE} ${_fixhtml_py} >${_output}
      DEPENDS ${_input}
    )
  endif()
  add_custom_target("${_lang}-html-handbook" ALL DEPENDS ${_output})
  install(FILES ${_output} DESTINATION ${WITH_DOCDIR})
endmacro()

# Create handbook for KDE 5.
# Replace meinproc5 with wrapper to change DTD version, then generate KDE help.
macro(kf5_create_handbook _docbook _lang)
  cmake_minimum_required(VERSION 3.3)
  find_package(ECM REQUIRED NO_MODULE)
  set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
  include(KDEInstallDirs)
  find_package(KF${QT_MAJOR_VERSION} REQUIRED COMPONENTS DocTools)
  set(KDOCTOOLS_MEINPROC_EXECUTABLE ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/kf5-meinproc-wrapper.py)
  if(QT_VERSION_MAJOR EQUAL 6 AND NOT HTML_INSTALL_DIR)
    # TODO KF6: If there something defined?
    set(HTML_INSTALL_DIR "${WITH_DATAROOTDIR}/doc/HTML")
  endif()
  kdoctools_create_handbook(${_docbook} INSTALL_DESTINATION ${HTML_INSTALL_DIR}/${_lang} SUBDIR kid3)
endmacro()

if(UNIX)
  if(NOT WITH_NO_MANCOMPRESS)
    find_program(GZIP_EXECUTABLE gzip)
  endif()

  # Create man page from DocBook file in language directory
  macro(kid3_create_manpage _docbook _lang)
    get_filename_component(_input ${_docbook} ABSOLUTE)
    set(_output ${CMAKE_CURRENT_BINARY_DIR}/${_lang}/kid3.1)
    set(_fixdocbook_py ${CMAKE_CURRENT_SOURCE_DIR}/fixmandocbook.py)
    set(_docbook_xsl ${DOCBOOK_XSL_DIR}/manpages/docbook.xsl)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_lang})
    add_custom_command(OUTPUT ${_output}
      COMMAND
        ${PYTHON_EXECUTABLE} ${_fixdocbook_py} ${_input} |
        ${XSLTPROC} --maxdepth 20000 --novalid --nonet ${_docbook_xsl} -
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_lang}
      DEPENDS ${_input}
    )
    if(WITH_NO_MANCOMPRESS)
      set(_gzExt "")
      set(_output_gz ${_output})
    else()
      set(_gzExt ".gz")
      set(_output_gz ${_output}${_gzExt})
      add_custom_command(OUTPUT ${_output_gz}
        COMMAND ${GZIP_EXECUTABLE} -cn ${_output} >${_output_gz}
        DEPENDS ${_output}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_lang}
      )
    endif()
    add_custom_target("${_lang}-man-handbook" ALL DEPENDS ${_output_gz})
    if(_lang STREQUAL "en")
      set(_install_dir ${WITH_MANDIR}/man1)
    else()
      set(_install_dir ${WITH_MANDIR}/${_lang}/man1)
    endif()
    install(FILES ${_output_gz} DESTINATION ${_install_dir})
    if(BUILD_QT_APP)
      install(CODE
        "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink kid3.1${_gzExt} \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${_install_dir}/kid3-qt.1${_gzExt}\")")
    endif()
    if(BUILD_CLI_APP)
      install(CODE
        "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink kid3.1${_gzExt} \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${_install_dir}/kid3-cli.1${_gzExt}\")")
    endif()
  endmacro()
endif()

if(BUILD_KDE_APP)
  file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kf5-meinproc-wrapper.py
       CONTENT "#!/usr/bin/env python3
import sys, os, tempfile
args = sys.argv[1:]
docbook = args.pop()
out_fh = tempfile.NamedTemporaryFile(mode='wb')
with open(docbook, 'rb') as in_fh:
    for line in in_fh:
        line = line.replace(
            b'\"-//KDE//DTD DocBook XML V4.2-Based Variant V1.1//EN\" \"dtd/kdex.dtd\"',
            b'\"-//KDE//DTD DocBook XML V4.5-Based Variant V1.1//EN\" \"dtd/kdedbx45.dtd\"'
                  ).replace(
            b'<!ENTITY doublehyphen \\'--\\'>',
            b'<!ENTITY doublehyphen \\'--\\'>' +
            b'\\n  <!ENTITY Debian \\'<acronym><trademark class=\"registered\">Debian</trademark></acronym>\\'>' +
            b'\\n  <!ENTITY macOS \\'<trademark class=\"registered\">macOS</trademark>\\'>' +
            b'\\n  <!ENTITY JSON \"<acronym>JSON</acronym>\">' +
            b'\\n  <!ENTITY javascript \"<application>JavaScript</application>\">' +
            b'\\n  <!ENTITY Del	\"<keycap>Del</keycap>\">' +
            b'\\n  <!ENTITY Up \"<keysym>Up</keysym>\">'
            b'\\n  <!ENTITY Down \"<keysym>Down</keysym>\">')
        out_fh.write(line)
out_fh.flush()
args.append(out_fh.name)
os.execv('$<TARGET_FILE:KF${QT_MAJOR_VERSION}::meinproc${QT_MAJOR_VERSION}>', args)
")
endif()

list(APPEND _languages en)
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
  set(_po_dir ${CMAKE_CURRENT_SOURCE_DIR}/po)
  message(WARNING "Using custom translations from ${_po_dir}")
else()
  set(_po_dir ${CMAKE_SOURCE_DIR}/po)
endif()
foreach(_lang ${_languages})
  if(_lang STREQUAL "en")
    set(_dir ${CMAKE_CURRENT_SOURCE_DIR}/en)
  else()
    set(_dir ${_po_dir}/${_lang}/docs/kid3)
  endif()

  if(BUILD_KDE_APP)
    kf5_create_handbook(${_dir}/index.docbook ${_lang})
  endif()

  if(BUILD_QT_APP)
    kid3_create_handbook(${_dir}/index.docbook ${_lang})
  endif()

  if(UNIX)
    kid3_create_manpage(${_dir}/index.docbook ${_lang})
  endif()
endforeach()
