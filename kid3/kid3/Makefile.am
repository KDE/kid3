# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
#SUBDIRS =

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kid3

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# the library search path. 
kid3_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
kid3_LDADD   = $(LIB_KFILE)

# which sources should be compiled for kid3
kid3_SOURCES = filelist.cpp framelist.cpp genres.cpp id3form.cpp kid3.cpp main.cpp mp3file.cpp standardtags.cpp configdialog.h configdialog.cpp formatconfig.h formatconfig.cpp formatbox.h formatbox.cpp importdialog.h importdialog.cpp importselector.h importselector.cpp importparser.h importparser.cpp generalconfig.h generalconfig.cpp

# these headers are automatically built
BUILT_SOURCES =
CLEANFILES =

# these are the headers for your project that won't be installed
noinst_HEADERS = configdialog.h filelist.h formatbox.h formatconfig.h framelist.h generalconfig.h genres.h id3form.h importdialog.h importparser.h importselector.h kid3.h mp3file.h standardtags.h

# additional files in distribution
EXTRA_DIST = kid3.desktop kid3ui.rc hi16-app-kid3.png hi32-app-kid3.png hi48-app-kid3.png kid3.h.both kid3.h.kde kid3.h.qtonly de_qt.po ru_qt.po kid3.dsp kid3.dsw kid3win.rc kid3.ico kid3.mak config.mk

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kid3/
	$(INSTALL_DATA) $(srcdir)/kid3ui.rc $(kde_datadir)/kid3/kid3ui.rc
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/hi16-app-kid3.png $(kde_icondir)/hicolor/16x16/apps/kid3.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/hi32-app-kid3.png $(kde_icondir)/hicolor/32x32/apps/kid3.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/apps/
	$(INSTALL_DATA) $(srcdir)/hi48-app-kid3.png $(kde_icondir)/hicolor/48x48/apps/kid3.png

uninstall-local:
	-rm -f $(kde_datadir)/kid3/kid3ui.rc
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kid3.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/kid3.png
	-rm -f $(kde_icondir)/hicolor/48x48/apps/kid3.png

# create kid3.h, the original file kid3.h.both cannot be run through moc
# because of ifdefs.
kid3.h: kid3.h.both
	(echo -e "/* Generated from kid3.h.both */\n#ifndef KID3_H\n#define KID3_H\n#include \"config.h\"\n#ifdef CONFIG_USE_KDE\n#include <kmainwindow.h>\n#else\n#include <qmainwindow.h>\n#endif\n"; \
	$(CC) -x c++ -E -P $(DEFAULT_INCLUDES) $(INCLUDES) -DKMAINWINDOW_H -DQMAINWINDOW_H kid3.h.both; \
	echo "#endif") >kid3.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kid3.pot

# this is where the desktop file will go 
desktopdir   = $(kde_appsdir)/Multimedia
desktop_DATA = kid3.desktop
