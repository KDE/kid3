# Download the translations directly from SVN.
# Usage: cmake -P download-pos.cmake LANGUAGES
# CMAKE_ARGV0, CMAKE_ARGV1, CMAKE_ARGV2 are
# /path/to/cmake, -P, /path/to/download-pos.cmake
set(_languages ${CMAKE_ARGV3})
set(_urlprefix "https://websvn.kde.org/*checkout*/trunk/l10n-kf5")
set(_podir "${CMAKE_CURRENT_LIST_DIR}/po")
set(_hdr "User-Agent: Mozilla/5.0")
set(_url "${_urlprefix}/templates/messages/kid3/kid3_qt.pot")
message(STATUS "Downloading po")
file(DOWNLOAD ${_url} "${_podir}/kid3_qt.pot"
  HTTPHEADER "${_hdr}"
  STATUS _status)
if(NOT _status MATCHES "^0;")
  message(FATAL_ERROR "${_url}: ${_status}")
endif()
foreach(_lang ${_languages})
  set(_url "${_urlprefix}/${_lang}/messages/kid3/kid3_qt.po")
  file(DOWNLOAD ${_url} "${_podir}/${_lang}/kid3_qt.po"
    HTTPHEADER "${_hdr}"
    STATUS _status)
  if(NOT _status MATCHES "^0;")
    message(FATAL_ERROR "${_url}: ${_status}")
  endif()
endforeach()
