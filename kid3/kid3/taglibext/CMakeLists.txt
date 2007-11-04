if (HAVE_TAGLIB)
  include_directories(..)
  set(taglibextsrc
      urllinkframe.cpp unsynchronizedlyricsframe.cpp generalencapsulatedobjectframe.cpp
      speex/speexfile.cpp speex/speexproperties.cpp speex/taglib_speexfiletyperesolver.cpp
      trueaudio/taglib_trueaudiofiletyperesolver.cpp
      trueaudio/ttafile.cpp trueaudio/ttaproperties.cpp
      wavpack/taglib_wavpackfiletyperesolver.cpp wavpack/wvfile.cpp
      wavpack/wvproperties.cpp
      aac/aacfiletyperesolver.cpp mp2/mp2filetyperesolver.cpp)
  add_library(taglibext STATIC ${taglibextsrc})
endif (HAVE_TAGLIB)
