#!/usr/bin/env python3

import fileinput
import sys
import os


def fix_docbook(lang):
    scriptdir = os.path.dirname(os.path.abspath(__file__)).replace('\\', '/')
    for line in fileinput.input(mode='rb'):
        line = line\
          .replace(b'"-//KDE//DTD DocBook XML V4.5-Based Variant V1.1//EN" "dtd/kdedbx45.dtd" [',
                   b'"-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd" [\n' +
                   b"  <!ENTITY % fromkdoctools SYSTEM '" + scriptdir.encode() + b"/fromkdoctools.ent'>\n" +
                   b"  %fromkdoctools;\n" +
                   b"  <!ENTITY language '" + lang.encode() + b"'>")\
          .replace(b'ufleisch@', b'ufleisch at ')\
          .replace(b'&FDLNotice;',
                   b'<para><ulink url="http://www.gnu.org/licenses/licenses.html#FDL">FDL</ulink></para>')\
          .replace(b'&underFDL;',
                   b'<para><ulink url="http://www.gnu.org/licenses/licenses.html#FDL">FDL</ulink></para>')\
          .replace(b'&underGPL;',
                   b'<para><ulink url="http://www.gnu.org/licenses/licenses.html#GPL">GPL</ulink></para>')\
          .replace(b'&documentation.index;', b'')
        os.write(sys.stdout.fileno(), line)

if __name__ == '__main__':
    lang = 'en'
    if len(sys.argv) > 1:
        lang = sys.argv[1]
        if os.path.isfile(lang):
            lang = os.path.split(os.path.dirname(lang))[1]
            if lang == 'kid3':
                lang = os.path.normpath(sys.argv[1]).split(os.path.sep)[-4]
        else:
            del sys.argv[1]
    fix_docbook(lang)
