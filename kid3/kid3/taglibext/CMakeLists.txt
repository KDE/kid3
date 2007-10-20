if (HAVE_TAGLIB)
  include_directories(..)
  set(taglibextsrc
      urllinkframe.cpp unsynchronizedlyricsframe.cpp generalencapsulatedobjectframe.cpp)
  add_library(taglibext STATIC ${taglibextsrc})
endif (HAVE_TAGLIB)
