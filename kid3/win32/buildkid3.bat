%MSYSDIR%\bin\perl ../kid3-qt/configure-kid3-qt.pl --generate-ts --prefix= --with-bindir= --with-datarootdir= --with-docdir= --with-translationsdir= --without-musicbrainz --enable-gcc-pch
mkdir kid3win
make && make install INSTALL_ROOT=%cd%\kid3win
