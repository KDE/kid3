%MSYSDIR%\bin\perl ../configure-kid3-qt.pl --generate-ts --prefix= --bindir= --datarootdir= --docdir= --translationsdir= --without-musicbrainz --enable-pch
mkdir kid3win
make && make install INSTALL_ROOT=%cd%\kid3win
