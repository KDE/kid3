#!/usr/bin/env python3

import fileinput
import sys
import os


def fix_docbook(lang):
    scriptdir = os.path.dirname(os.path.abspath(__file__))
    for line in fileinput.input():
        line = line\
          .replace('"-//KDE//DTD DocBook XML V4.5-Based Variant V1.1//EN" "dtd/kdedbx45.dtd" [',
                   '"-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd" [\n' +
                   "  <!ENTITY % fromkdoctools SYSTEM '" + scriptdir + "/fromkdoctools.ent'>\n" +
                   "  %fromkdoctools;\n" +
                   "  <!ENTITY language '" + lang + "'>")\
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
    lang = 'en'
    if len(sys.argv) > 1:
        lang = sys.argv[1]
        if os.path.isfile(lang):
            lang = os.path.split(os.path.dirname(lang))[1]
        else:
            del sys.argv[1]
    fix_docbook(lang)
