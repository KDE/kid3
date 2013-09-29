#!/usr/bin/perl -n
s/"-\/\/KDE\/\/DTD DocBook XML V4.2-Based Variant V1.1\/\/EN" "dtd\/kdex.dtd"/"-\/\/OASIS\/\/DTD DocBook XML V4.2\/\/EN" "http:\/\/www.oasis-open.org\/docbook\/xml\/4.2\/docbookx.dtd"/;
s/<!ENTITY % German "INCLUDE">/<!ENTITY language "de">/;
s/<!ENTITY % English "INCLUDE">/<!ENTITY language "en">/;
s/ufleisch@/ufleisch at /g;
s/&FDLNotice;/<para><ulink url="http:\/\/www.gnu.org\/licenses\/licenses.html#FDL">FDL<\/ulink><\/para>/g;
s/&underFDL;/<para><ulink url="http:\/\/www.gnu.org\/licenses\/licenses.html#FDL">FDL<\/ulink><\/para>/g;
s/&underGPL;/<para><ulink url="http:\/\/www.gnu.org\/licenses\/licenses.html#GPL">GPL<\/ulink><\/para>/g;
s/&documentation.index;//g;
s/<book /<article /;
s/<\/book>/<\/refentry>\n<\/article>/;
s/<bookinfo/<articleinfo/;
s/<\/bookinfo>/<\/articleinfo>\n\n<refentry id="kid3">\n\n/;
s/<!-- *<refmeta>/<refmeta>/;
s/<\/refnamediv> *-->/<\/refnamediv>/;
s/^<!--<refsynopsisdiv>-->.*$/<refsynopsisdiv>/;
s/^<!--<\/refsynopsisdiv>-->.*$/<\/refsynopsisdiv>/;
s/<preface/<refsect1/;
s/<\/preface/<\/refsect1/;
s/<chapter/<refsect1/;
s/<\/chapter/<\/refsect1/;
s/<sect1/<refsect2/;
s/<\/sect1/<\/refsect2/;
s/<sect2/<refsect3/;
s/<\/sect2/<\/refsect3/;
s/<appendix/<refsect1/;
s/<\/appendix/<\/refsect1/;
print;
