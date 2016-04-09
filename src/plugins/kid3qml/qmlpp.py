#!/usr/bin/env python
#
# qmlpp - QML and JavaScript preprocessor
# Based on https://katastrophos.net/andre/blog/2013/09/20/qml-preprocessor-the-qnd-and-kiss-way/
# Converted to Python because it is already required to build Kid3.
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version
# 2.1 as published by the Free Software Foundation.

import sys
import os
import re


class QmlPreprocessor(object):
    def __init__(self, rewrite_qtquick_version, defines, process_inline):
        self.rewrite_qtquick_version = rewrite_qtquick_version
        self.definesre = re.compile(defines)
        self.process_inline = process_inline
        self.importre = re.compile(r'import QtQuick\s*\d.\d')
        self.uncommentre = re.compile(r'^(\s*)(//)?(.*)//@(.*)')
        self.deflinere = re.compile(r'^(\s*)(.*)//(@.*)')

    def preprocess_file(self, fn):
        lines = []
        with open(fn) as fh:
            for line in fh:
                if '//!noRewrite' not in line:
                    line = self.importre.sub('import QtQuick ' +
                                             self.rewrite_qtquick_version, line)
                line = self.uncommentre.sub(r'\1\3//@\4', line)
                if not self.definesre.search(line):
                    line = self.deflinere.sub(r'\1//\2//\3', line)
                if self.process_inline:
                    lines.append(line)
                else:
                    sys.stdout.write(line)
        if self.process_inline:
            fh = open(fn, 'w')
            fh.writelines(lines)
            fh.close()

    def preprocess_directory(self, dirname):
        for root, dirs, files in os.walk(dirname):
            try:
                dirs.remove('.git')
            except ValueError:
                pass
            try:
                dirs.remove('.svn')
            except ValueError:
                pass
            for fn in files:
                if fn.endswith(('.qml', '.js')):
                    self.preprocess_file(os.path.join(root, fn))

    def run(self, inp):
        if os.path.isfile(inp) or inp == '-':
            self.preprocess_file(inp)
        elif os.path.isdir(inp):
            if not self.process_inline:
                raise Exception('Please specify -i when trying to preprocess '
                                'a whole directory recursively.')
            self.preprocess_directory(inp)
        else:
            raise Exception('Please specify a valid file or directory.')


def usage():
    print("""usage: %s [options] <filename JS or QML or directoryname>

OPTIONS:
   -h                Show this message.
   -q <major.minor>  The version of QtQuick to rewrite to. Example: -q 2.1
   -d <defines>      The defines to set, separated by |. Example: -d "@QtQuick1|@Meego|@Debug"
   -i                Modify file in-place instead of dumping to stdout.
""" % sys.argv[0])


def main(argv):
    import getopt
    try:
        opts, args = getopt.getopt(argv, "hiq:d:")
    except getopt.GetoptError as err:
        print(str(err))
        usage()
        sys.exit(1)
    rewrite_qt_quick_version = ''
    defines = ''
    process_inline = False
    for o, a in opts:
        if o == '-q':
            rewrite_qt_quick_version = a
        elif o == '-d':
            defines = a
        elif o == '-i':
            process_inline = True
        elif o == '-h':
            usage()
            sys.exit(0)
    inp = args[0] if args else '.'

    # Shortcuts for Kid3: 4, 5 and U for Qt4, Qt5 and Ubuntu.
    if inp == '4':
        rewrite_qt_quick_version = '1.1'
        defines = '@QtQuick1|@!Ubuntu'
        process_inline = True
        inp = '.'
    elif inp == '5':
        rewrite_qt_quick_version = '2.2'
        defines = '@QtQuick2|@!Ubuntu'
        process_inline = True
        inp = '.'
    elif inp == 'U':
        rewrite_qt_quick_version = '2.2'
        defines = '@QtQuick2|@Ubuntu'
        process_inline = True
        inp = '.'

    qmlpp = QmlPreprocessor(rewrite_qt_quick_version, defines, process_inline)
    try:
        qmlpp.run(inp)
    except Exception as e:
        print(str(e))
        usage()
        sys.exit(1)

if __name__ == '__main__':
    main(sys.argv[1:])
