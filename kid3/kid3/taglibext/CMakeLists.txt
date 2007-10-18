if (HAVE_TAGLIB)
  include_directories(..)
  add_library(taglibext STATIC urllinkframe.cpp unsynchronizedlyricsframe.cpp)
endif (HAVE_TAGLIB)
