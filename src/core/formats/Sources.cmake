set(formats_SRCS
  formats/attributedata.cpp
  formats/flacfile.cpp
  formats/m4afile.cpp
  formats/mp3file.cpp
  formats/oggfile.cpp
  formats/taglibfile.cpp
  formats/vcedit.c
)

set(formats_MOC_HDRS)

if (HAVE_TAGLIB)
  set(formats_SRCS ${formats_SRCS}
    formats/taglibext/aac/aacfiletyperesolver.cpp
    formats/taglibext/mp2/mp2filetyperesolver.cpp
  )

  if (TAGLIB_VERSION STREQUAL "1.4")
    set(formats_SRCS ${formats_SRCS}
      formats/taglibext/urllinkframe.cpp
      formats/taglibext/unsynchronizedlyricsframe.cpp
      formats/taglibext/generalencapsulatedobjectframe.cpp
      formats/taglibext/speex/speexfile.cpp
      formats/taglibext/speex/speexproperties.cpp
      formats/taglibext/speex/taglib_speexfiletyperesolver.cpp
      formats/taglibext/trueaudio/taglib_trueaudiofiletyperesolver.cpp
      formats/taglibext/trueaudio/ttafile.cpp
      formats/taglibext/trueaudio/ttaproperties.cpp
      formats/taglibext/wavpack/taglib_wavpackfiletyperesolver.cpp
      formats/taglibext/wavpack/wvfile.cpp
      formats/taglibext/wavpack/wvproperties.cpp
    )
  endif (TAGLIB_VERSION STREQUAL "1.4")
endif (HAVE_TAGLIB)
