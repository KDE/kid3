#!/usr/bin/env python

import fileinput
import sys


def fix_desktop():
    for line in fileinput.input():
        line = line\
          .replace('Name=Kid3', 'Name=Kid3-qt')\
          .replace('Exec=kid3', 'Exec=kid3-qt')\
          .replace('Icon=kid3', 'Icon=kid3-qt')\
          .replace('Categories=Qt;KDE', 'Categories=Qt')\
          .replace('StartupWMClass=kid3', 'StartupWMClass=kid3-qt')
        if not line.startswith('X-DocPath'):
            sys.stdout.write(line)

if __name__ == '__main__':
    fix_desktop()
