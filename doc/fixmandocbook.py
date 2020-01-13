#!/usr/bin/env python

import fileinput
import re
import sys


def fix_man_docbook():
    for line in fileinput.input():
        line = line\
          .replace('"-//KDE//DTD DocBook XML V4.5-Based Variant V1.1//EN" "dtd/kdedbx45.dtd" [',
                   '"-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd" [\n' +
                   "  <!ENTITY Alt '<keycap>Alt</keycap>'>\n  <!ENTITY Ctrl '<keycap>Ctrl</keycap>'>\n" +
                   "  <!ENTITY DBus 'D-Bus'>\n  <!ENTITY GUI 'GUI'>\n" +
                   "  <!ENTITY HTML 'HTML'>\n  <!ENTITY JSON 'JSON'>\n  <!ENTITY Linux 'Linux'>\n  <!ENTITY Qt 'Qt'>\n" +
                   "  <!ENTITY Shift '<keycap>Shift</keycap>'>\n  <!ENTITY URL 'URL'>\n  <!ENTITY Windows 'Windows'>\n" +
                   "  <!ENTITY XML 'XML'>\n  <!ENTITY eg 'e.g.'>\n  <!ENTITY etc 'etc.'>\n  <!ENTITY ie 'i.e.'>\n" +
                   "  <!ENTITY kde 'KDE'>\n  <!ENTITY macOS 'macOS'>")\
          .replace('<!ENTITY % German "INCLUDE">',
                   '<!ENTITY language "de">')\
          .replace('<!ENTITY % English "INCLUDE">',
                   '<!ENTITY language "en">')\
          .replace('ufleisch@', 'ufleisch at ')\
          .replace('&FDLNotice;',
                   '<para><ulink url="http://www.gnu.org/licenses/licenses.html#FDL">FDL</ulink></para>')\
          .replace('&underFDL;',
                   '<para><ulink url="http://www.gnu.org/licenses/licenses.html#FDL">FDL</ulink></para>')\
          .replace('&underGPL;',
                   '<para><ulink url="http://www.gnu.org/licenses/licenses.html#GPL">GPL</ulink></para>')\
          .replace('&documentation.index;', '')\
          .replace('<book ', '<article ')\
          .replace('</book>', '</refentry>\n</article>')\
          .replace('<bookinfo', '<articleinfo')\
          .replace('</bookinfo>',
                   '</articleinfo>\n\n<refentry id="kid3">\n\n')\
          .replace('<preface', '<refsect1')\
          .replace('</preface', '</refsect1')\
          .replace('<chapter', '<refsect1')\
          .replace('</chapter', '</refsect1')\
          .replace('<sect1', '<refsect2')\
          .replace('</sect1', '</refsect2')\
          .replace('<sect2', '<refsect3')\
          .replace('</sect2', '</refsect3')\
          .replace('<appendix', '<refsect1')\
          .replace('</appendix', '</refsect1')
        line = re.sub(r'^<!--change manpage(.*)-->.*$', r'\1', line)
        line = re.sub(r'^<!--begin manpage include$',
                       r'<!--begin manpage include-->', line)
        line = re.sub(r'^end manpage include-->$',
                   r'<!--end manpage include-->', line)
        line = re.sub(r'^<!--begin manpage ignore-->$',
                       r'<!--begin manpage ignore', line)
        line = re.sub(r'^<!--end manpage ignore-->$',
                           r'end manpage ignore-->', line)
        sys.stdout.write(line)

if __name__ == '__main__':
    fix_man_docbook()
