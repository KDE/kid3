# Download the translations directly from SVN.
# Usage: cmake -P download-pos.cmake LANGUAGES
# CMAKE_ARGV0, CMAKE_ARGV1, CMAKE_ARGV2 are
# /path/to/cmake, -P, /path/to/download-pos.cmake
set(_languages ${CMAKE_ARGV3})
set(_urlprefix "https://websvn.kde.org/*checkout*/trunk/l10n-kf5")
set(_podir "${CMAKE_CURRENT_LIST_DIR}/po")
set(_docsdir "${CMAKE_CURRENT_LIST_DIR}/docs")
set(_hdr "User-Agent: Mozilla/5.0")
set(_url "${_urlprefix}/templates/docmessages/kid3/kid3.pot")
message(STATUS "Downloading po, docs")
file(DOWNLOAD ${_url} "${_podir}/kid3.pot"
  HTTPHEADER "${_hdr}"
  STATUS _status)
if(NOT _status MATCHES "^0;")
  message(FATAL_ERROR "${_url}: ${_status}")
endif()
foreach(_lang ${_languages})
  set(_url "${_urlprefix}/${_lang}/docmessages/kid3/kid3.po")
  file(DOWNLOAD ${_url} "${_podir}/${_lang}/kid3.po"
    HTTPHEADER "${_hdr}"
    STATUS _status)
  if(NOT _status MATCHES "^0;")
    message(FATAL_ERROR "${_url}: ${_status}")
  endif()
  set(_url "${_urlprefix}/${_lang}/docs/kid3/kid3/index.docbook")
  file(DOWNLOAD ${_url} "${_docsdir}/${_lang}/index.docbook"
    HTTPHEADER "${_hdr}"
    STATUS _status)
  if(NOT _status MATCHES "^0;")
    message(FATAL_ERROR "${_url}: ${_status}")
  endif()
endforeach()
