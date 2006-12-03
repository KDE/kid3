#!/bin/sh
# Build without KDE

TOPDIR=$(dirname $0)
QT4=n
DESTDIR=./kid3qt
for opt in $@; do
  if test "$opt" = "-qt4"; then
    QT4=y
  else
    DESTDIR=$opt
  fi
done

KDEDIR=/usr
# SUSE: KDEDIR=/opt/kde3
INSTALL="/usr/bin/install -c -p"

finish_html() {
# quoted the following characters: # -> \#, / -> \/
perl -ne "s/ufleisch@/ufleisch at /g; s/common\/fdl-license.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#FDL/g; s/common\/gpl-license.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#GPL/g; s/common\/fdl-translated.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#FDL/g; s/common\/gpl-translated.html/http:\/\/www.gnu.org\/licenses\/licenses.html\#GPL/g; s/<div class=\"toc\">.+?<\/div><div class=\"sect1\">/<div class=\"sect1\">/g; print"
}

if test ! -f Makefile; then
  $TOPDIR/configure --without-kde
  if test "$QT4" = "y"; then
    make qt4-Makefile
  fi
fi
make

$INSTALL -d $DESTDIR
$INSTALL ./kid3/kid3 $DESTDIR/kid3
cat $TOPDIR/po/de.po $TOPDIR/kid3/de_qt.po >tmp.po
msg2qm tmp.po $DESTDIR/kid3_de.qm
cat $TOPDIR/po/ru.po $TOPDIR/kid3/ru_qt.po >tmp.po
msg2qm tmp.po $DESTDIR/kid3_ru.qm
cat $TOPDIR/po/es.po $TOPDIR/kid3/es_qt.po >tmp.po
msg2qm tmp.po $DESTDIR/kid3_es.qm
cat $TOPDIR/po/fr.po $TOPDIR/kid3/fr_qt.po >tmp.po
msg2qm tmp.po $DESTDIR/kid3_fr.qm
rm -f tmp.po
if test "$QT4" = "y"; then
  qm2ts $DESTDIR/kid3_de.qm && rm $DESTDIR/kid3_de.qm && lrelease $DESTDIR/kid3_de.ts && rm $DESTDIR/kid3_de.ts
  qm2ts $DESTDIR/kid3_ru.qm && rm $DESTDIR/kid3_ru.qm && lrelease $DESTDIR/kid3_ru.ts && rm $DESTDIR/kid3_ru.ts
  qm2ts $DESTDIR/kid3_es.qm && rm $DESTDIR/kid3_es.qm && lrelease $DESTDIR/kid3_es.ts && rm $DESTDIR/kid3_es.ts
  qm2ts $DESTDIR/kid3_fr.qm && rm $DESTDIR/kid3_fr.qm && lrelease $DESTDIR/kid3_fr.ts && rm $DESTDIR/kid3_fr.ts
  $INSTALL -m 644 /usr/share/qt4/translations/qt_de.qm $DESTDIR/qt_de.qm
  $INSTALL -m 644 /usr/share/qt4/translations/qt_ru.qm $DESTDIR/qt_ru.qm
  $INSTALL -m 644 /usr/share/qt4/translations/qt_fr.qm $DESTDIR/qt_fr.qm
fi

SGML_CATALOG_FILES=$KDEDIR/share/apps/ksgmltools2/customization/catalog xsltproc --catalogs $KDEDIR/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl $TOPDIR/doc/en/index.docbook | finish_html >$DESTDIR/kid3_en.html
SGML_CATALOG_FILES=$KDEDIR/share/apps/ksgmltools2/customization/catalog xsltproc --catalogs $KDEDIR/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl $TOPDIR/doc/de/index.docbook | finish_html >$DESTDIR/kid3_de.html
