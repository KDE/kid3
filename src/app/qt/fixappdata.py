#!/usr/bin/env python3

import fileinput
import sys


def fix_appdata():
    for line in fileinput.input(mode='rb'):
        line = line.replace(b'kid3.desktop</id>', b'kid3_qt.desktop</id>') \
                   .replace(b'kid3.desktop', b'kid3-qt.desktop')
        sys.stdout.buffer.write(line)

if __name__ == '__main__':
    fix_appdata()
