#!/usr/bin/perl -n
s/"-\/\/KDE\/\/DTD DocBook XML V4.2-Based Variant V1.1\/\/EN" "dtd\/kdex.dtd"/"-\/\/OASIS\/\/DTD DocBook XML V4.2\/\/EN" "http:\/\/www.oasis-open.org\/docbook\/xml\/4.2\/docbookx.dtd"/;
s/<!ENTITY % German "INCLUDE">/<!ENTITY language "de">/;
s/<!ENTITY % English "INCLUDE">/<!ENTITY language "en">/;
s/ufleisch@/ufleisch at /g;
s/&FDLNotice;/<para><ulink url="http:\/\/www.gnu.org\/licenses\/licenses.html#FDL">FDL<\/ulink><\/para>/g;
s/&underFDL;/<para><ulink url="http:\/\/www.gnu.org\/licenses\/licenses.html#FDL">FDL<\/ulink><\/para>/g;
s/&underGPL;/<para><ulink url="http:\/\/www.gnu.org\/licenses\/licenses.html#GPL">GPL<\/ulink><\/para>/g;
s/&documentation.index;//g;
print;
