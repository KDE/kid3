# Download the translations directly from SVN.
# Usage: cmake -P download-pos.cmake LANGUAGES
# CMAKE_ARGV0, CMAKE_ARGV1, CMAKE_ARGV2 are
# /path/to/cmake, -P, /path/to/download-pos.cmake
set(_languages ${CMAKE_ARGV3})
if(NOT CMAKE_VERSION VERSION_LESS "3.11")
  include(FetchContent)
  FetchContent_Populate(l10nkf5_docmessages_pot
    SVN_REPOSITORY svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/templates/docmessages/kid3
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/po")
  foreach(_lang ${_languages})
    string(REPLACE "@" "" _lang_simplified ${_lang})
    FetchContent_Populate(l10nkf5_docmessages_${_lang_simplified}
      SVN_REPOSITORY svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/${_lang}/docmessages/kid3
      SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/po/${_lang}")
    FetchContent_Populate(l10nkf5_docs_${_lang}
      SVN_REPOSITORY svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/${_lang}/docs/kid3/kid3
      SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/docs/${_lang}")
  endforeach()
endif()
