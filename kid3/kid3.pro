SOURCES	+= filelist.cpp framelist.cpp genres.cpp kid3.cpp main.cpp mp3file.cpp standardtags.cpp 
HEADERS	+= filelist.h framelist.h genres.h kid3.h mp3file.h standardtags.h 
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= id3form.ui
TEMPLATE	=app
CONFIG	+= qt warn_on release
LIBS	+= -lqt-mt -lid3
LANGUAGE	= C++
