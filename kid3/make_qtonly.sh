#!/bin/sh
# Build without KDE

KDEDIR=/opt/kde3
QTDIR=/usr/lib/qt3
INSTALL="/usr/bin/install -c -p"

finish_html() {
# quoted the following characters: # -> \#, / -> \/
perl -ne "s/ufleisch@/ufleisch at /g; s/common\/fdl-license.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#FDL/g; s/common\/gpl-license.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#GPL/g; s/common\/fdl-translated.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#FDL/g; s/common\/gpl-translated.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#GPL/g; s/<div class=\"toc\">.+?<\/div><div class=\"sect1\">/<div class=\"sect1\">/g; print"
}

DESTDIR=$1
echo $DESTDIR
test -z $DESTDIR && DESTDIR=./kid3qt
echo $DESTDIR

./configure --without-kde
make
$INSTALL -d $DESTDIR
$INSTALL ./kid3/kid3 $DESTDIR/kid3
cat po/de.po kid3/de_qt.po >tmp.po
$QTDIR/bin/msg2qm tmp.po $DESTDIR/kid3_de.qm
rm tmp.po
SGML_CATALOG_FILES=$KDEDIR/share/apps/ksgmltools2/customization/catalog xsltproc --catalogs $KDEDIR/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl doc/en/index.docbook | finish_html >$DESTDIR/kid3_en.html
SGML_CATALOG_FILES=$KDEDIR/share/apps/ksgmltools2/customization/catalog xsltproc --catalogs $KDEDIR/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl doc/de/index.docbook | finish_html >$DESTDIR/kid3_de.html
