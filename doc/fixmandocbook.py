#!/usr/bin/env python

import fileinput
import re
import sys


def fix_man_docbook():
    for line in fileinput.input():
        line = line\
          .replace('"-//KDE//DTD DocBook XML V4.2-Based Variant V1.1//EN" "dtd/kdex.dtd"',
                   '"-//OASIS//DTD DocBook XML V4.2//EN" "http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd"')\
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
