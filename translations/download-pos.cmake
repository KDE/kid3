# Download the translations directly from SVN.
# Usage: cmake -P download-pos.cmake LANGUAGES
# CMAKE_ARGV0, CMAKE_ARGV1, CMAKE_ARGV2 are
# /path/to/cmake, -P, /path/to/download-pos.cmake
set(_languages ${CMAKE_ARGV3})
if(NOT CMAKE_VERSION VERSION_LESS "3.11")
  include(FetchContent)
  FetchContent_Populate(l10nkf5_messages_pot
    SVN_REPOSITORY svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/templates/messages/kid3
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/po")
  foreach(_lang ${_languages})
    string(REPLACE "@" "" _lang_simplified ${_lang})
    FetchContent_Populate(l10nkf5_messages_${_lang_simplified}
      SVN_REPOSITORY svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/${_lang}/messages/kid3
      SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/po/${_lang}")
  endforeach()
endif()
