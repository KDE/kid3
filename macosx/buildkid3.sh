#!/bin/sh
perl ../kid3-qt/configure-kid3-qt.pl --generate-ts --prefix= --with-bindir= --with-datarootdir=Kid3.app/Contents/Resources --with-docdir=Kid3.app/Contents/Resources --with-translationsdir=Kid3.app/Contents/Resources --without-musicbrainz --without-phonon --enable-gcc-pch --with-db2html="xsltproc --novalid --nonet" --with-xsl-stylesheet=$HOME/docbook-xsl-1.72.0/html/docbook.xsl --with-extra-includes=/usr/local/include --with-taglib-includes=-I/usr/local/include/taglib --with-extra-libs=-L/usr/local/lib
make

echo "Creating bundle"
rm -rf Kid3.app
cp -R kid3/kid3.app Kid3.app
cp -f ../kid3/hisc-app-kid3.svgz doc/en/kid3_en.html doc/de/kid3_de.html po/*.qm Kid3.app/Contents/Resources/
cp -f ../kid3/hi48-app-kid3.png Kid3.app/Contents/Resources/kid3.png
cp -f kid3.icns Kid3.app/Contents/Resources/
cp -f Info.plist Kid3.app/Contents/

echo "Deploying Qt framework"
mkdir -p Kid3.app/Contents/Frameworks/QtXml.framework/Versions/4

for module in Xml Gui Network Core; do
  mkdir -p Kid3.app/Contents/Frameworks/Qt${module}.framework/Versions/4
  lipo /Library/Frameworks/Qt${module}.framework/Versions/4/Qt${module} \
    -output Kid3.app/Contents/Frameworks/Qt${module}.framework/Versions/4/Qt${module} -thin i386
  install_name_tool \
    -id @executable_path/../Frameworks/Qt${module}.framework/Versions/4/Qt${module} \
    Kid3.app/Contents/Frameworks/Qt${module}.framework/Versions/4/Qt${module}
  install_name_tool \
    -change Qt${module}.framework/Versions/4/Qt${module} \
    @executable_path/../Frameworks/Qt${module}.framework/Versions/4/Qt${module} \
    Kid3.app/Contents/MacOS/kid3
done

for module in Xml Gui Network; do
  install_name_tool \
    -change QtCore.framework/Versions/4/QtCore \
    @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
    Kid3.app/Contents/Frameworks/Qt${module}.framework/Versions/4/Qt${module}
done

mkdir -p Kid3.app/Contents/PlugIns/imageformats/
lipo /Developer/Applications/Qt/plugins/imageformats/libqjpeg.dylib \
 -output Kid3.app/Contents/PlugIns/imageformats/libqjpeg.dylib -thin i386

for module in Gui Core; do
  install_name_tool \
    -change Qt${module}.framework/Versions/4/Qt${module} \
    @executable_path/../Frameworks/Qt${module}.framework/Versions/4/Qt${module} \
    Kid3.app/Contents/PlugIns/imageformats/libqjpeg.dylib
done
