# Makefile for kid3

CONFIGMK_PATH = $(wildcard config.mk)
ifneq (,$(findstring config.mk,$(CONFIGMK_PATH)))
# read CONFIG_... defines
include config.mk
endif

ifndef QTDIR
QTDIR    = /usr/lib/qt3
endif
ifndef KDEDIR
KDEDIR   = /opt/kde3
endif

# Output directories
OBJECTS_DIR = .obj
MOC_DIR = .moc

# Compiler, tools and options
CC       = gcc
CXX      = g++
CFLAGS   = -pipe -Wall -W -mcpu=i486 -march=i486
ifdef CONFIG_DEBUG
CFLAGS  += -O -g
else
CFLAGS  += -O2 -DNO_DEBUG -DQT_NO_DEBUG
endif
CFLAGS  += -I$(KDEDIR)/include -I$(QTDIR)/include -I$(MOC_DIR) -I.
CXXFLAGS = $(CFLAGS)
LINK     = g++
LFLAGS   = -Wl,-rpath,$(QTDIR)/lib  -L$(QTDIR)/lib -lqt-mt -lid3
ifdef CONFIG_USE_KDE
LFLAGS  += -L$(KDEDIR)/lib -lkdecore -lkdeui -lkio
ifneq (,$(wildcard $(KDEDIR)/lib/libkfile.so))
# library for file dialog in KDE2
LFLAGS  += -lkfile
endif
endif
AR       = ar cqs
RANLIB   = 
MOC      = $(QTDIR)/bin/moc
TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f
COPY_FILE= $(COPY) -p
COPY_DIR = $(COPY) -pR
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv
PERL = /usr/bin/perl
INSTALL = /usr/bin/install -c -p
# quoted the following characters: # -> \#, $ -> $$
# make C defines from Makefile definitions
MK2H = $(PERL) -ne "print if (s/^([^\#][^=\s]*)[\s=]+(.+)$$/\#define \1 \2/)"
# prepend .o filenames by OBJECTS_DIR
ADDOBJDIR = $(PERL) -ne "s/(\w+\.o)/$(OBJECTS_DIR)\/\1/; print"
# quoted the following characters: # -> \#, / -> \/
FINISH_HTML = $(PERL) -ne "s/ufleisch@/ufleisch at /g; s/common\/fdl-license.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#FDL/g; s/common\/gpl-license.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#GPL/g; s/common\/fdl-translated.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#FDL/g; s/common\/gpl-translated.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#GPL/g; s/<div class=\"toc\">.+?<\/div><div class=\"sect1\">/<div class=\"sect1\">/g; print"
# Files
HEADERS = filelist.h framelist.h genres.h kid3_1.h kid3_2.h mp3file.h \
          standardtags.h id3form.h
SOURCES = filelist.cpp framelist.cpp genres.cpp kid3.cpp main.cpp \
          mp3file.cpp standardtags.cpp id3form.cpp
OBJECTS  = $(SOURCES:%.cpp=$(OBJECTS_DIR)/%.o)
SRCMOC   = $(MOC_DIR)/moc_framelist.cpp $(MOC_DIR)/moc_kid3.cpp \
           $(MOC_DIR)/moc_id3form.cpp
OBJMOC   = $(SRCMOC:$(MOC_DIR)/%.cpp=$(OBJECTS_DIR)/%.o)
DIST     = AUTHORS COPYING INSTALL LICENSE README ChangeLog \
           Makefile config.mk kid3.mak kid3.spec \
           hi16-app-kid3.png hi32-app-kid3.png hi48-app-kid3.png \
           kid3.desktop kid3ui.rc
DISTNAME = kid3-$(CONFIG_VERSION:"%"=%)
TARGET   = kid3

# Implicit rules
$(OBJECTS_DIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJECTS_DIR)/%.o: $(MOC_DIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(MOC_DIR)/moc_%.cpp: %.h
	$(MOC) $< -o $@

# Build rules
all: autoconf.h .depend $(TARGET) docs i18n

ifneq (,$(findstring config.mk,$(CONFIGMK_PATH)))

# generate autoconf.h from config.mk
autoconf.h: config.mk
	$(MK2H) $< >$@

endif

$(TARGET): $(OBJECTS) $(OBJMOC) 
	$(LINK) $(LFLAGS) -o $(TARGET) $^

mocables: $(SRCMOC)

mocclean:
	-$(DEL_FILE) $(OBJMOC)
	-$(DEL_FILE) $(SRCMOC)

clean: mocclean
	-$(DEL_FILE) $(OBJECTS) 
	-$(DEL_FILE) *~ core *.core
	-$(DEL_FILE) kid3.h
	-$(DEL_FILE) autoconf.h

distclean: clean
	-$(DEL_FILE) $(TARGET)
	-$(DEL_FILE) TAGS
	-$(DEL_FILE) .depend
	-$(DEL_FILE) po/de/kid3.mo
	-$(DEL_FILE) kid3_de.qm
	-$(DEL_FILE) doc/en/index.cache.bz2
	-$(DEL_FILE) doc/de/index.cache.bz2

FORCE:

####### Compile

# moc can't handle ifdefs, so we create a partially preprocessed file before
kid3.h: kid3_1.h kid3_2.h autoconf.h
	echo -e "#ifndef KID3_H\n#define KID3_H" >kid3.h
	cat kid3_1.h >>kid3.h
	$(CXX) -E -C -P -x c++ $(CXXFLAGS) kid3_2.h >>kid3.h
	echo "#endif" >>kid3.h

# Internationalization
ifdef CONFIG_USE_KDE
i18n: po/de/kid3.mo
else
i18n: kid3_de.qm
endif

po/de/kid3.po: $(SOURCES)
	xgettext --omit-header -C -ki18n -kI18N_NOOP -ktr2i18n -ktranslate -x/opt/kde3/include/kde.pot  -otmp.po $(SOURCES)
	msgmerge -U $@ tmp.po
	$(DEL_FILE) tmp.po

po/de/kid3.mo: po/de/kid3.po
	msgfmt -o $@ $<

# Translation for Qt (non-KDE) application
kid3_de.qm: po/de/kid3.po po/de/kid3qt.po
	cat po/de/kid3.po po/de/kid3qt.po >tmp.po; \
	$(QTDIR)/bin/msg2qm tmp.po kid3_de.qm; \
	$(DEL_FILE) tmp.po

# Documentation
ifdef CONFIG_USE_KDE
docs: doc/en/index.cache.bz2 doc/de/index.cache.bz2
else
docs: kid3_en.html kid3_de.html
endif

doc/en/index.cache.bz2: doc/en/index.docbook
	$(KDEDIR)/bin/meinproc --check --cache doc/en/index.cache.bz2 doc/en/index.docbook

doc/de/index.cache.bz2: doc/de/index.docbook
	$(KDEDIR)/bin/meinproc --check --cache doc/de/index.cache.bz2 doc/de/index.docbook

kid3_en.html: doc/en/index.docbook
	SGML_CATALOG_FILES=$(KDEDIR)/share/apps/ksgmltools2/customization/catalog xsltproc --catalogs $(KDEDIR)/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl doc/en/index.docbook | $(FINISH_HTML) >kid3_en.html

kid3_de.html: doc/de/index.docbook
	SGML_CATALOG_FILES=$(KDEDIR)/share/apps/ksgmltools2/customization/catalog xsltproc --catalogs $(KDEDIR)/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl doc/de/index.docbook | $(FINISH_HTML) >kid3_de.html

# Installation
ifdef CONFIG_USE_KDE
install: kdeinstall
uninstall: kdeuninstall
else
ifndef DESTDIR
DESTDIR = ./kid3qt
endif
install: qtinstall
uninstall: qtuninstall
endif

kdeinstall: all
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/bin
	$(INSTALL) ./kid3 $(DESTDIR)/$(KDEDIR)/bin/kid3
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/applnk/Multimedia
	$(INSTALL) -m 644 ./kid3.desktop $(DESTDIR)/$(KDEDIR)/share/applnk/Multimedia/kid3.desktop
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/16x16/apps
	$(INSTALL) -m 644 ./hi16-app-kid3.png $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/16x16/apps/kid3.png
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/32x32/apps
	$(INSTALL) -m 644 ./hi32-app-kid3.png $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/32x32/apps/kid3.png
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/48x48/apps
	$(INSTALL) -m 644 ./hi48-app-kid3.png $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/48x48/apps/kid3.png
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/apps/kid3
	$(INSTALL) -m 644 ./kid3ui.rc $(DESTDIR)/$(KDEDIR)/share/apps/kid3/kid3ui.rc
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3
	$(INSTALL) -m 644 doc/en/index.cache.bz2 $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3/index.cache.bz2
	$(INSTALL) -m 644 doc/en/index.docbook $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3/index.docbook
	$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3/common
	$(SYMLINK) $(KDEDIR)/share/doc/HTML/en/common $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3/common
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3
	$(INSTALL) -m 644 doc/de/index.cache.bz2 $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3/index.cache.bz2
	$(INSTALL) -m 644 doc/de/index.docbook $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3/index.docbook
	$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3/common
	$(SYMLINK) $(KDEDIR)/share/doc/HTML/de/common $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3/common
	$(INSTALL) -d $(DESTDIR)/$(KDEDIR)/share/locale/de/LC_MESSAGES
	$(INSTALL) -m 644 po/de/kid3.mo $(DESTDIR)/$(KDEDIR)/share/locale/de/LC_MESSAGES/kid3.mo

kdeuninstall:
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/bin/kid3
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/applnk/Multimedia/kid3.desktop
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/16x16/apps/kid3.png
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/32x32/apps/kid3.png
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/icons/hicolor/48x48/apps/kid3.png
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/apps/kid3/kid3ui.rc
	-$(DEL_DIR)  $(DESTDIR)/$(KDEDIR)/share/apps/kid3
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3/index.cache.bz2
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3/index.docbook
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3/common
	-$(DEL_DIR)  $(DESTDIR)/$(KDEDIR)/share/doc/HTML/en/kid3
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3/index.cache.bz2
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3/index.docbook
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3/common
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3/common
	-$(DEL_DIR)  $(DESTDIR)/$(KDEDIR)/share/doc/HTML/de/kid3
	-$(DEL_FILE) $(DESTDIR)/$(KDEDIR)/share/locale/de/LC_MESSAGES/kid3.mo

qtinstall: all
	$(INSTALL) -d $(DESTDIR)
	$(INSTALL) ./kid3 $(DESTDIR)/kid3
	$(INSTALL) -m 644 ./kid3_de.qm $(DESTDIR)/kid3_de.qm
	$(INSTALL) -m 644 ./kid3_en.html $(DESTDIR)/kid3_en.html
	$(INSTALL) -m 644 ./kid3_de.html $(DESTDIR)/kid3_de.html

qtuninstall:
	-$(DEL_FILE) $(DESTDIR)/kid3 $(DESTDIR)/kid3_de.qm $(DESTDIR)/kid3_en.html
	-$(DEL_DIR) $(DESTDIR)

# Tags
TAGS: autoconf.h
	ctags -e $(HEADERS) autoconf.h $(SOURCES)

# Distribution
# As .html and .qm files cannot be generated from Windows,
# they are included in the distribution.
dist: kid3_en.html kid3_de.html kid3_de.qm
	mkdir $(DISTNAME) $(DISTNAME)/$(MOC_DIR)
	mkdir $(DISTNAME)/$(OBJECTS_DIR)
	mkdir $(DISTNAME)/doc  $(DISTNAME)/doc/en $(DISTNAME)/doc/de
	mkdir $(DISTNAME)/po $(DISTNAME)/po/de
	cp $(DIST) $(SOURCES) $(HEADERS) $(FORMS) $(DISTNAME)/
	cp doc/en/index.docbook $(DISTNAME)/doc/en/
	cp doc/de/index.docbook $(DISTNAME)/doc/de/
	cp po/de/kid3.po po/de/kid3qt.po $(DISTNAME)/po/de/
	cp kid3_en.html kid3_de.html $(DISTNAME)/
	cp kid3_de.qm $(DISTNAME)/
	tar czf $(DISTNAME).tgz $(DISTNAME)/
	rm -rf $(DISTNAME)/

# Dependencies which are needed before automatic generation of dependencies
$(MOC_DIR)/moc_framelist.cpp: framelist.h

$(MOC_DIR)/moc_kid3.cpp: kid3.h autoconf.h config.mk

$(MOC_DIR)/moc_id3form.cpp: id3form.h

# Automatic generation of dependencies
depend .depend dep: $(SRCMOC)
	$(CC) $(CFLAGS) -MM -MG $(SOURCES) $(SRCMOC) | $(ADDOBJDIR) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
