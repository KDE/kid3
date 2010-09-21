#!/usr/bin/perl -W
# Configuration of Kid3 build.
# Used to generate all necessary files to build kid3 without KDE.
# Can be used for basic configuration when no configure script
# is available (e.g. on Windows) or called at the end of a
# configure script with the options --from-configure --generate-ts.

use strict;
use File::Basename;
use File::Spec;
use File::Copy;

my $topdir = File::Spec->rel2abs(dirname($0). "/..");
my $lupdate_cmd = "lupdate";

# Read all source files given in the parameter list and copy them to the
# current directory, replaceing i18n by tr and I18N_NOOP by QT_TRANSLATE_NOOP.
sub createTranslateSources(@)
{
	my @sources = @_;
	foreach my $file (@sources) {
		open IF, $file or die "Could not open $file: $!\n";
		my $outfn = basename($file);
		open OF, ">$outfn" or die "Could not create $outfn: $!\n";
		while (<IF>) {
			s/i18n\(/tr(/g;
			s/KCM_i18n1\(([^,]+), ([^)]+)\)/tr($1).arg($2)/g;
			s/KCM_i18n2\(([^,]+), ([^,]+), ([^)]+)\)/tr($1).arg($2).arg($3)/g;
			s/I18N_NOOP\(/QT_TRANSLATE_NOOP("\@default", /g;
			print OF $_;
		}				
		close OF;
		close IF;
	}
}

# Read all translations from a .po file, fill them into an associative
# array.
sub getPoTranslations($)
{
	my $fn = shift;
	my $msgid;
	my $msgstr;
	my $in_msgid = 0;
	my $in_msgstr = 0;
	my %trans;
	open IF, $fn or die "Could not open $fn: $!\n";
	while (<IF>) {
		s/\r\n/\n/;
		if (/^msgid "(.*)"$/) {
			if ($msgid) {
				$trans{$msgid} = $msgstr;
			}
			$msgid = $1;
			$msgstr = "";
			$in_msgid = 1;
			$in_msgstr = 0;
		}
		if (/^msgstr "(.*)"$/) {
			$msgstr = $1;
			$in_msgid = 0;
			$in_msgstr = 1;
		}
		if (/^"(.+)"$/) {
			if ($in_msgid) {
				$msgid .= $1;
			} elsif ($in_msgstr) {
				$msgstr .= $1;
			}
		}
	}
	close IF;
	if ($msgid) {
		$trans{$msgid} = $msgstr;
	}
	return %trans;
}

# Set the translations in a .ts file replacing & by &amp;, < by &lt; and > by &gt;.
sub setTsTranslations($$%)
{
	my $infn = shift;
	my $outfn = shift;
	my %trans = @_;
	my $source;
	my $translation;
	my $in_source = 0;
	open IF, $infn or die "Could not open $infn: $!\n";
	open OF, ">$outfn" or die "Could not create $outfn: $!\n";
	while (<IF>) {
		s/\r\n/\n/;
		if (/<source>(.*)<\/source>/) {
			$source = $1;
			$in_source = 0;
		} elsif (/<source>(.*)$/) {
			$source = $1;
			$in_source = 1;
		} elsif ($in_source) {
			if (/^(.*)<\/source>/) {
				$source .= "\n$1";
				$in_source = 0;
			} else {
				my $line = $_;
				chomp $line;
				$source .= "\n$line";
			}
		} elsif (/<translation/) {
			$source =~ s/&amp;/&/g;
			$source =~ s/&lt;/</g;
			$source =~ s/&gt;/>/g;
			$source =~ s/\n/\\n/g;
			if (exists $trans{$source}) {
				$translation = $trans{$source};
				$translation =~ s/&/&amp;/g;
				$translation =~ s/</&lt;/g;
				$translation =~ s/>/&gt;/g;
				$translation =~ s/\\"/&quot;/g;
				$translation =~ s/\\n/\n/g;
				s/ type="unfinished"//;
				s/<\/translation>/$translation<\/translation>/;
			} else {
				print "Could not find translation for \"$source\"\n";
			}
		}
		print OF $_;
	}
	close OF;
	close IF;
}

# Generate .ts files from .po files.
sub generateTs()
{
	my @pofiles = glob "$topdir/po/*.po";
	my @languages = map { /^.*\W(\w+)\.po$/ } @pofiles;
	my $tmpdir = ".tsdir";
	mkdir $tmpdir unless -d $tmpdir;
	mkdir "po" unless -d "po";
	chdir $tmpdir or die "Could not change to $tmpdir: $!\n";
	my @sources = glob "$topdir/kid3/*.cpp";
	createTranslateSources(@sources);
	system "$lupdate_cmd " . join ' ', glob "*.cpp" . " -ts ". join ' ', map { "kid3_". $_ . ".ts" } @languages;
	foreach my $lang (@languages) { 
		setTsTranslations("kid3_$lang.ts", "../po/kid3_$lang.ts",
											getPoTranslations("$topdir/po/$lang.po"));
	}
	unlink <*>;
	chdir "..";
	rmdir $tmpdir;
}


my $have_vorbis = 1;
my $have_flac = 1;
my $have_flac_picture = 1;
my $have_id3lib = 1;
my $have_taglib = 1;
my $taglib_includes = "-I/usr/include/taglib";
my $have_mp4v2 = 1;
my $have_mp4v2_mp4v2_h = 1;
my $have_qtdbus = 0;
my $have_phonon = 1;
my $have_tunepimp = 5;
my $qmake_cmd = "qmake";
my $enable_pch = 0;
my $enable_debug = 0;
my $prefix;
my $bindir;
my $datarootdir;
my $docdir;
my $translationsdir;
my $extra_includes;
my $extra_libs;
my $extra_defines;
my $extra_cxxflags;
my $db2html;
my $perl_cmd;
my $xsl_stylesheet;
my $from_configure;
my $generate_ts;

while (my $opt = shift) {
  if ($opt eq "--without-vorbis") {
		$have_vorbis = 0;
  } elsif ($opt eq "--without-flac") {
		$have_flac = 0;
		$have_flac_picture = 0;
  } elsif ($opt eq "--without-flac-picture") {
		$have_flac_picture = 0;
  } elsif ($opt eq "--without-id3lib") {
		$have_id3lib = 0;
  } elsif ($opt eq "--without-taglib") {
		$have_taglib = 0;
	} elsif (substr($opt, 0, 23) eq "--with-taglib-includes=") {
		$taglib_includes = substr($opt, 23);
  } elsif ($opt eq "--without-mp4v2") {
		$have_mp4v2 = 0;
		$have_mp4v2_mp4v2_h = 0;
  } elsif ($opt eq "--with-dbus") {
		$have_qtdbus = 1;
  } elsif ($opt eq "--without-phonon") {
		$have_phonon = 0;
	} elsif ($opt eq "--without-musicbrainz") {
		$have_tunepimp = 0;
	} elsif (substr($opt, 0, 19) eq "--with-musicbrainz=") {
		$have_tunepimp = substr($opt, 19);
	} elsif (substr($opt, 0, 13) eq "--with-qmake=") {
		$qmake_cmd = substr($opt, 13);
	} elsif ($opt eq "--enable-pch") {
		$enable_pch = 1;
	} elsif ($opt eq "--enable-debug") {
		$enable_debug = 1;
	} elsif (substr($opt, 0, 9) eq "--prefix=") {
		$prefix = substr($opt, 9);
	} elsif (substr($opt, 0, 14) eq "--with-bindir=") {
		$bindir = substr($opt, 14);
	} elsif (substr($opt, 0, 19) eq "--with-datarootdir=") {
		$datarootdir = substr($opt, 19);
	} elsif (substr($opt, 0, 14) eq "--with-docdir=") {
		$docdir = substr($opt, 14);
	} elsif (substr($opt, 0, 23) eq "--with-translationsdir=") {
		$translationsdir = substr($opt, 23);
	} elsif (substr($opt, 0, 22) eq "--with-extra-includes=") {
		$extra_includes = substr($opt, 22);
	} elsif (substr($opt, 0, 18) eq "--with-extra-libs=") {
		$extra_libs = substr($opt, 18);
	} elsif (substr($opt, 0, 21) eq "--with-extra-defines=") {
		$extra_defines = substr($opt, 21);
	} elsif (substr($opt, 0, 22) eq "--with-extra-cxxflags=") {
		$extra_cxxflags = substr($opt, 22);
	} elsif (substr($opt, 0, 15) eq "--with-db2html=") {
		$db2html = substr($opt, 15);
	} elsif (substr($opt, 0, 22) eq "--with-xsl-stylesheet=") {
		$xsl_stylesheet = substr($opt, 22);
	} elsif (substr($opt, 0, 16) eq "--with-perl-cmd=") {
		$perl_cmd = substr($opt, 16);
	} elsif ($opt eq "--from-configure") {
		$from_configure = 1;
	} elsif ($opt eq "--generate-ts") {
		$generate_ts = 1;
	} elsif ($opt eq "-h" or $opt eq "--help") {
		print "Usage: $0 [OPTION]\nOptions [default]:\n";
		print "  --prefix=PREFIX        install prefix [/usr/local]\n";
		print "  --with-bindir=DIR      user executables [PREFIX/bin]\n";
		print "  --with-datarootdir=DIR data root [PREFIX/share]\n";
		print "  --with-docdir=DIR      documentation [DATAROOTDIR/doc/kid3-qt]\n";
		print "  --with-translationsdir=DIR translations [DATAROOTDIR/kid3-qt/translations]\n";
		print "  --with-extra-includes=DIR adds non standard include paths\n";
		print "  --with-extra-libs=LIB  adds non standard library options\n";
		print "  --with-extra-defines=D adds non standard defines\n";
		print "  --with-extra-cxxflags=F adds non standard compiler options\n";
		print "  --with-db2html=PROGRAM Docbook to HTML command (e.g. xsltproc, jw, xalan)\n";
		print "  --with-xsl-stylesheet=P path to docbook.xsl\n";
		print "  --with-perl-cmd=PROGRAM perl command\n";
		print "  --without-musicbrainz  build without MusicBrainz\n";
		print "  --with-musicbrainz=VER build with MusicBrainz version [5]\n";
		print "  --without-taglib       build without taglib\n";
		print "  --with-taglib-includes=I taglib includes [-I/usr/include/taglib]\n";
		print "  --without-mp4v2        build without mp4v2\n";
		print "  --without-id3lib       build without id3lib\n";
		print "  --without-vorbis       build without ogg/vorbis\n";
		print "  --without-flac         build without FLAC\n";
		print "  --without-flac-picture build without FLAC picture support\n";
		print "  --with-dbus            build with QtDBus\n";
		print "  --enable-gcc-pch       enable precompiled headers\n";
		print "  --enable-debug         enables debug symbols\n";
		print "  --with-qmake=PROGRAM   qmake command [qmake]\n";
    print "  --from-configure       started from configure, ignore settings above,\n";
    print "                         do not generate config.h, config.pri\n";
		print "  --generate-ts          generate ts from po translations\n";
		exit 0;
	}
}
if (!defined $prefix) {
 $prefix = "/usr/local";
}
if (!defined $bindir) {
	$bindir = $prefix . "/bin";
}
if (!defined $datarootdir) {
	$datarootdir = $prefix . "/share";
}
if (!defined $docdir) {
	$docdir = $datarootdir . "/doc/kid3-qt";
}
if (!defined $translationsdir) {
	$translationsdir = $datarootdir . "/kid3-qt/translations";
}

my $fn;
if ($from_configure) {
	# started from configure => ignore settings above, get them from configure
	$prefix = "";
	$bindir = "";
	$datarootdir = "";
	$docdir = "";
	$translationsdir = "";
	$have_id3lib = 0;
	$have_vorbis = 0;
	$have_flac = 0;
	$have_taglib = 0;
	$have_mp4v2 = 0;
	$have_mp4v2_mp4v2_h = 0;
	$have_qtdbus = 0;
	$have_phonon = 0;
	$have_tunepimp = 0;
	$qmake_cmd = "";
	$lupdate_cmd = "";
	$enable_pch = 0;
	$enable_debug = 0;

	$fn = "kid3/config.h";
	if (open IF, "$fn") {
		while (<IF>) {
			s/\r\n/\n/;
			if (/^#define CFG_PREFIX "(.*)"$/) {
				$prefix = $1;
			} elsif (/^#define CFG_BINDIR "(.*)"$/) {
				$bindir = $1;
			} elsif (/^#define CFG_DATAROOTDIR "(.*)"$/) {
				$datarootdir = $1;
			} elsif (/^#define CFG_DOCDIR "(.*)"$/) {
				$docdir = $1;
			} elsif (/^#define CFG_TRANSLATIONSDIR "(.*)"$/) {
				$translationsdir = $1;
			} elsif (/^#define HAVE_ID3LIB (\d+)$/) {
				$have_id3lib = $1;
			} elsif (/^#define HAVE_VORBIS (\d+)$/) {
				$have_vorbis = $1;
			} elsif (/^#define HAVE_FLAC (\d+)$/) {
				$have_flac = $1;
			} elsif (/^#define HAVE_TAGLIB (\d+)$/) {
				$have_taglib = $1;
			} elsif (/^#define HAVE_MP4V2 (\d+)$/) {
				$have_mp4v2 = $1;
			} elsif (/^#define HAVE_MP4V2_MP4V2_H (\d+)$/) {
				$have_mp4v2_mp4v2_h = $1;
			} elsif (/^#define HAVE_QTDBUS (\d+)$/) {
				$have_qtdbus = $1;
			} elsif (/^#define HAVE_PHONON (\d+)$/) {
				$have_phonon = $1;
			} elsif (/^#define HAVE_TUNEPIMP (\d+)$/) {
				$have_tunepimp = $1;
			} elsif (/^#define CFG_QMAKE "(.*)"$/) {
				$qmake_cmd = $1;
			} elsif (/^#define CFG_LUPDATE "(.*)"$/) {
				$lupdate_cmd = $1;
			} elsif (/^#define GCC_PCH (\d+)$/) {
				$enable_pch = $1;
			} elsif (/^#define CFG_DEBUG (\d+)$/) {
				$enable_debug = $1;
			}
		}
		close IF;
	}
}

my $config_h = "#define VERSION \"1.5\"\n";
my $config_pri;
if ($prefix) {
	$config_h .= "#define CFG_PREFIX \"$prefix\"\n";
	$config_pri .= "CFG_PREFIX = $prefix\n";
}
if ($bindir) {
	$config_h .= "#define CFG_BINDIR \"$bindir\"\n";
	$config_pri .= "CFG_BINDIR = $bindir\n";
}
if ($datarootdir) {
	$config_h .= "#define CFG_DATAROOTDIR \"$datarootdir\"\n";
	$config_pri .= "CFG_DATAROOTDIR = $datarootdir\n";
}
if ($docdir) {
	$config_h .= "#define CFG_DOCDIR \"$docdir\"\n";
	$config_pri .= "CFG_DOCDIR = $docdir\n";
}
if ($translationsdir) {
	$config_h .= "#define CFG_TRANSLATIONSDIR \"$translationsdir\"\n";
	$config_pri .= "CFG_TRANSLATIONSDIR = $translationsdir\n";
}
if ($taglib_includes) {
	$config_pri .= "TAGLIB_INCLUDES = $taglib_includes\n";
}
if ($extra_includes) {
	$config_pri .= "CFG_EXTRA_INCLUDES = $extra_includes\n";
}
if ($extra_libs) {
	$config_pri .= "CFG_EXTRA_LIBS = $extra_libs\n";
}
if ($extra_defines) {
	$config_pri .= "CFG_EXTRA_DEFINES = $extra_defines\n";
}
if ($extra_cxxflags) {
	$config_pri .= "CFG_EXTRA_CXXFLAGS = $extra_cxxflags\n";
}
if ($db2html) {
	$config_pri .= "CFG_DB2HTML = $db2html\n";
}
if ($xsl_stylesheet) {
	$config_pri .= "CFG_XSL_STYLESHEET = $xsl_stylesheet\n";
}
if ($perl_cmd) {
	$config_pri .= "CFG_PERL_CMD = $perl_cmd\n";
}
$config_pri .= "CFG_LIBS = ";

my $allsys_h = <<"EOF";
/* automatically generated by configure-kid3-qt.pl */
#ifndef ALLSYS_H
#define ALLSYS_H
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utime.h>
#include <sys/stat.h>
#ifdef __cplusplus
#include <cmath>
#include <cstdio>
EOF

if ($have_id3lib) {
	$config_h .= "#define HAVE_ID3LIB $have_id3lib\n";
	$config_pri .= "-lid3 ";
	if ($^O eq "darwin") {
		$config_pri .= "-liconv -lz ";
	}
#  $allsys_h .= "#include <id3.h>\n#include <id3/tag.h>\n";
}
if ($have_vorbis) {
	$config_h .= "#define HAVE_VORBIS $have_vorbis\n";
	$config_pri .= "-logg -lvorbisfile -lvorbis ";
	$allsys_h .= "#include <ogg/ogg.h>\n#include <vorbis/codec.h>\n#include <vorbis/vorbisfile.h>\n";
}
if ($have_flac) {
	$config_h .= "#define HAVE_FLAC $have_flac\n";
	$config_pri .= "-lFLAC++ -lFLAC ";
	$allsys_h .= "#include <FLAC++/metadata.h>\n";
}
if ($have_flac_picture) {
	$config_h .= "#define HAVE_FLAC_PICTURE $have_flac_picture\n";
}
if ($have_taglib) {
	$config_h .= "#define HAVE_TAGLIB $have_taglib\n";
	$config_pri .= "-ltag ";
}
if ($have_mp4v2) {
	$config_h .= "#define HAVE_MP4V2 $have_mp4v2\n";
	$config_pri .= "-lmp4v2 ";
	$config_pri .= "-lwsock32 " if $^O eq "MSWin32" or $^O eq "msys";
	if ($have_mp4v2_mp4v2_h) {
		$config_h .= "#define HAVE_MP4V2_MP4V2_H $have_mp4v2_mp4v2_h\n";
	}
}
if ($have_tunepimp) {
	$config_h .= "#define HAVE_TUNEPIMP $have_tunepimp\n";
	$config_pri .= "-ltunepimp ";
	if ($have_tunepimp == 5) {
		$allsys_h .= "#include <tunepimp-0.5/tp_c.h>\n";
	} else {
		$allsys_h .= "#include <tunepimp/tp_c.h>\n";
	}
}
if ($have_qtdbus) {
	$config_h .= "#define HAVE_QTDBUS $have_qtdbus\n";
	$config_pri .= "\nHAVE_QTDBUS = $have_qtdbus";
}
if ($have_phonon) {
	$config_h .= "#define HAVE_PHONON $have_phonon\n";
	$config_pri .= "\nHAVE_PHONON = $have_phonon";
}

$config_pri .= "\nCFG_CONFIG = " .
	($enable_debug ? "debug" : "release") .
	($enable_pch ? " precompile_header\nCFG_PRECOMPILED_HEADER = allsys.h\n" : "\n");
$allsys_h .= <<"EOF";
#include <qaction.h>
#include <qapplication.h>
#include <qbitarray.h>
#include <qbuffer.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qdialog.h>
#include <qdir.h>
#include <qdom.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qprinter.h>
#include <qprocess.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextbrowser.h>
#include <qtextcodec.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qtranslator.h>
#include <qurl.h>
#include <qvalidator.h>
#endif
#endif
EOF

mkdir "kid3" unless -d "kid3";

if (!$from_configure) {
	$fn = "kid3/config.h";
	my $old_config_h;
	if (open IF, "$fn") {
		$old_config_h = join('', <IF>);
		close IF;
	}
	if ($old_config_h and $old_config_h eq $config_h) {
		print "keeping $fn\n";
	} else {
		open OF, ">$fn" or die "Cannot open $fn: $!\n";
		print "creating $fn\n";
		print OF "$config_h";
		close OF;
	}

	$fn = "config.pri";
	my $old_config_pri;
	if (open IF, "$fn") {
		$old_config_pri = join('', <IF>);
		close IF;
	}
	if ($old_config_pri and $old_config_pri eq $config_pri) {
		print "keeping $fn\n";
	} else {
		open OF, ">$fn" or die "Cannot open $fn: $!\n";
		print "creating $fn\n";
		print OF "$config_pri";
		close OF;
	}
}

if ($enable_pch) {
	$fn = "kid3/allsys.h";
	my $old_allsys_h;
	if (open IF, "$fn") {
		$old_allsys_h = join('', <IF>);
		close IF;
	}
	if ($old_allsys_h and $old_allsys_h eq $allsys_h) {
		print "keeping $fn\n";
	} else {
		open OF, ">$fn" or die "Cannot open $fn: $!\n";
		print "creating $fn\n";
		print OF "$allsys_h";
		close OF;
	}
}

if (open IF, "$topdir/kid3/kid3.desktop") {
  $fn = "kid3/kid3-qt.desktop";
	open OF, ">$fn" or die "Cannot open $fn: $!\n";
	while (<IF>) {
		s/^Name=Kid3$/Name=Kid3-qt/;
		s/^Exec=kid3/Exec=kid3-qt/;
		s/^Icon=kid3$/Icon=kid3-qt/;
		s/^Categories=Qt;KDE/Categories=Qt/;
		print OF $_ unless /^X-DocPath/;
	}
	print "creating $fn\n";
	close OF;
	close IF;
}

print "copying icons\n";
copy("$topdir/kid3/hi16-app-kid3.png", "kid3/hi16-app-kid3-qt.png");
copy("$topdir/kid3/hi32-app-kid3.png", "kid3/hi32-app-kid3-qt.png");
copy("$topdir/kid3/hi48-app-kid3.png", "kid3/hi48-app-kid3-qt.png");
copy("$topdir/kid3/hisc-app-kid3.svgz", "kid3/hisc-app-kid3-qt.svgz");

mkdir "doc" unless -d "doc";
$fn = "doc/fixdocbook.pl";
open OF, ">$fn" or die "Cannot open $fn: $!\n";
print OF <<"EOF";
#!/usr/bin/perl -n
s/"-\\/\\/KDE\\/\\/DTD DocBook XML V4.2-Based Variant V1.1\\/\\/EN" "dtd\\/kdex.dtd"/"-\\/\\/OASIS\\/\\/DTD DocBook XML V4.2\\/\\/EN" "http:\\/\\/www.oasis-open.org\\/docbook\\/xml\\/4.2\\/docbookx.dtd"/;
s/<!ENTITY % German "INCLUDE">/<!ENTITY language "de">/;
s/<!ENTITY % English "INCLUDE">/<!ENTITY language "en">/;
s/ufleisch@/ufleisch at /g;
s/&FDLNotice;/<para><ulink url="http:\\/\\/www.gnu.org\\/licenses\\/licenses.html#FDL">FDL<\\/ulink><\\/para>/g;
s/&underFDL;/<para><ulink url="http:\\/\\/www.gnu.org\\/licenses\\/licenses.html#FDL">FDL<\\/ulink><\\/para>/g;
s/&underGPL;/<para><ulink url="http:\\/\\/www.gnu.org\\/licenses\\/licenses.html#GPL">GPL<\\/ulink><\\/para>/g;
s/&documentation.index;//g;
print;
EOF
print "creating $fn\n";
close OF;

$fn = "doc/fixhtml.pl";
open OF, ">$fn" or die "Cannot open $fn: $!\n";
print OF <<"EOF";
#!/usr/bin/perl -n
s/^><TITLE\$/><meta http-equiv="content-type" content="text\\/html; charset=UTF-8"\\n><TITLE/ms;
s/<\\/title/<\\/title>\\
<style type="text\\/css">\\
body { font-family: Arial, Helvetica, sans-serif; color: #000000; background: #ffffff; }\\
h1, h2, h3, h4 { text-align: left; font-weight: bold; color: #f7800a; background: transparent; }\\
a:link { color: #0057ae; }\\
pre { display: block; color: #000000; background: #f9f9f9; border: #2f6fab dashed; border-width: 1px; overflow: auto; line-height: 1.1em; }\\
dt { font-weight: bold; color: #0057ae; }\\
p { text-align: justify; }\\
li { text-align: left; }\\
.guibutton, .guilabel, .guimenu, .guimenuitem { font-family: Arial, Helvetica, sans-serif; color: #000000; background: #dcdcdc; }\\
.application { font-weight: bold; }\\
.command { font-family: "Courier New", Courier, monospace; }\\
.filename { font-style: italic; }\\
<\\/style/i;
print;
EOF
print "creating $fn\n";
close OF;

if ($generate_ts) {
	print "creating .ts files\n";
	generateTs();
}

print "starting $qmake_cmd\n";
if ($^O eq "darwin") {
	system "$qmake_cmd -spec macx-g++ $topdir/kid3-qt.pro";
} else {
	system "$qmake_cmd $topdir/kid3-qt.pro";
	chdir "kid3";
	system "$qmake_cmd $topdir/kid3/kid3.pro";
	chdir "..";
	chdir "po";
	system "$qmake_cmd $topdir/po/po.pro";
	chdir "..";
	chdir "doc";
	mkdir "en" unless -d "en";
	mkdir "de" unless -d "de";
	chdir "en";
	system "$qmake_cmd $topdir/doc/en/en.pro";
	chdir "..";
	chdir "de";
	system "$qmake_cmd $topdir/doc/de/de.pro";
	chdir "..";
	chdir "..";
}

if ($enable_pch) {
# qmake < Qt 4.2 generates a dependency requiring the precompiled header
# to be in the source tree, this is fixed here.
	my $makefile_changed = 0;
	if (open IF, "kid3/Makefile") {
		if (open OF, ">kid3/Makefile.new") {
			while (<IF>) {
				if (s#^(.*\s)\S+kid3/(allsys\.h.*)$#$1$2#) {
					$makefile_changed = 1;
				}
				print OF $_;
			}
			close OF;
		}
		close IF;
	}
	if ($makefile_changed) {
		rename "kid3/Makefile", "kid3/Makefile.bak";
		rename "kid3/Makefile.new", "kid3/Makefile";
		print "fixing kid3/Makefile\n";
	}
}

print "\nStart make now\n";
