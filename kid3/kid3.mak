# Makefile for building kid3 using nmake, cl, link
# id3form.ui has to be compiled with uic of Qt3/Linux
# kid3_de.qm for German language strings has to be generated
# with Linux using the KDE tools.

# Compiler, tools and options
# directory containing id3lib.lib
ID3LIBDIR = .\id3lib\id3lib-3.8.0binaries\Release
# directory containing id3.h, id3\
ID3INCDIR = .\id3lib\include

CC	=	cl
CXX	=	cl
CFLAGS	=	-nologo -W3 -MD -O1 -DQT_DLL -DQT_THREAD_SUPPORT -DNO_DEBUG -DID3LIB_LINKOPTION=3 -GR
CXXFLAGS	=	$(CFLAGS)
INCPATH	=	-I"$(QTDIR)\include" -I"$(ID3INCDIR)" -I".\.ui" -I".\.moc" -I"."
LINK	=	link
LFLAGS	=	/NOLOGO /SUBSYSTEM:windows
LIBS	=	$(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib imm32.lib winmm.lib wsock32.lib $(ID3LIBDIR)\id3lib.lib
MOC	=	$(QTDIR)\bin\moc.exe
ZIP	=	zip -r -9

TAR      = tar -cf
GZIP     = gzip -9f
COPY     = copy
COPY_FILE= $(COPY)
COPY_DIR = $(COPY)
DEL_FILE = del
DEL_DIR  = rmdir
PERL     = perl
# quoted the following characters: # -> \x23, $ -> $$
# make C defines from Makefile definitions
MK2H = $(PERL) -ne "print if (s/^([^\x23][^=\s]*)[\s=]+(.+)$$/\x23define \1 \2/)"

# Files

HEADERS = filelist.h \
		framelist.h \
		genres.h \
		kid3.h \
		mp3file.h \
		standardtags.h
SOURCES = filelist.cpp \
		framelist.cpp \
		genres.cpp \
		kid3.cpp \
		main.cpp \
		mp3file.cpp \
		standardtags.cpp
OBJECTS = filelist.obj \
		framelist.obj \
		genres.obj \
		kid3.obj \
		main.obj \
		mp3file.obj \
		standardtags.obj \
		id3form.obj
FORMS = id3form.ui
UICDECLS = .ui\id3form.h
UICIMPLS = .ui\id3form.cpp
SRCMOC   = moc_framelist.cpp \
		moc_kid3.cpp \
		moc_id3form.cpp
OBJMOC = moc_framelist.obj \
		moc_kid3.obj \
		moc_id3form.obj
DIST	   = kid3.pro
DESTDIR  = 
TARGET   = kid3.exe

first: all
####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .c

.cpp.obj:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -Fo$@ $<

.cxx.obj:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -Fo$@ $<

.cc.obj:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -Fo$@ $<

.c.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

####### Build rules

all: autoconf.h $(TARGET)

# generate autoconf.h from config.mk
autoconf.h: config.mk
	$(MK2H) config.mk >$@

$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) 
	$(LINK) $(LFLAGS) /OUT:$(TARGET) @<<
	    $(OBJECTS) $(OBJMOC) $(LIBS)
<<

mocables: $(SRCMOC)

dist: 
	$(ZIP) kid3.zip kid3.pro $(SOURCES) $(HEADERS) $(DIST) $(INTERFACES)
#	@mkdir -p kid3 && $(COPY_FILE) --parents $(SOURCES) $(HEADERS) $(FORMS) $(DIST) kid3\ && $(COPY_FILE) --parents id3form.ui.h kid3/ && ( cd `dirname kid3` && $(TAR) kid3.tar kid3 && $(GZIP) kid3.tar ) && $(MOVE) `dirname kid3`/kid3.tar.gz . && $(DEL_DIR) kid3

mocclean:
	-$(DEL_FILE) $(OBJMOC)
	-$(DEL_FILE) $(SRCMOC)

clean: mocclean
	-$(DEL_FILE) $(OBJECTS) 
	-$(DEL_FILE) kid3.h
	-$(DEL_FILE) autoconf.h

distclean: clean
	-$(DEL_FILE) $(TARGET)


FORCE:

# Compile

# moc can't handle ifdefs, so we create a partially preprocessed file before
kid3.h: kid3_1.h kid3_2.h autoconf.h
	$(PERL) -e "print \"\x23ifndef KID3_H\n\x23define KID3_H\n\"" >kid3.h
	type kid3_1.h >>kid3.h
	$(CXX) /EP /C $(CXXFLAGS) kid3_2.h >>kid3.h
	$(PERL) -e "print \"\x23endif\n\"" >>kid3.h

filelist.obj: filelist.cpp mp3file.h \
		filelist.h

framelist.obj: framelist.cpp mp3file.h \
		framelist.h autoconf.h

genres.obj: genres.cpp genres.h

kid3.obj: kid3.cpp kid3.h \
		.ui\id3form.h \
		genres.h \
		framelist.h \
		filelist.h \
		standardtags.h \
		mp3file.h autoconf.h

main.obj: main.cpp kid3.h \
		filelist.h \
		standardtags.h \
		framelist.h \
		mp3file.h autoconf.h

mp3file.obj: mp3file.cpp standardtags.h \
		mp3file.h

standardtags.obj: standardtags.cpp standardtags.h

#.ui\id3form.h: id3form.ui 
#	$(UIC) id3form.ui -o .ui\id3form.h
#
#.ui\id3form.cpp: .ui\id3form.h id3form.ui 
#	$(UIC) id3form.ui -i id3form.h -o .ui\id3form.cpp

id3form.obj: .ui\id3form.cpp filelist.h \
		kid3.h \
		genres.h \
		id3form.ui.h \
		.ui\id3form.h autoconf.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -Foid3form.obj .ui\id3form.cpp

moc_framelist.obj: moc_framelist.cpp framelist.h 

moc_kid3.obj: moc_kid3.cpp kid3.h filelist.h \
		standardtags.h \
		framelist.h \
		mp3file.h autoconf.h

moc_id3form.obj: moc_id3form.cpp .ui\id3form.h 

moc_framelist.cpp: $(MOC) framelist.h
	$(MOC) framelist.h -o moc_framelist.cpp

moc_kid3.cpp: $(MOC) kid3.h
	$(MOC) kid3.h -o moc_kid3.cpp

moc_id3form.cpp: $(MOC) .ui\id3form.h
	$(MOC) .ui\id3form.h -o moc_id3form.cpp

# Install

!IF "$(CFG)" == ""
!MESSAGE No prefix specified. Defaulting to .\kid3win.
prefix = .\kid3win
!ENDIF

install: all 
	-mkdir $(prefix)
	-$(COPY) .\kid3.exe $(prefix)\kid3.exe
	-$(COPY) .\kid3_de.qm $(prefix)\kid3_de.qm
