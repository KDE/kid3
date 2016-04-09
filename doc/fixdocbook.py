#!/usr/bin/env python

import fileinput
import sys


def fix_docbook():
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
          .replace('&documentation.index;', '')
        sys.stdout.write(line)

if __name__ == '__main__':
    fix_docbook()
