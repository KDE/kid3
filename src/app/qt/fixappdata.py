#!/usr/bin/env python3

import fileinput
import sys


def fix_appdata():
    for line in fileinput.input(mode='rb'):
        if b'<project_group>KDE</project_group>' in line:
            continue
        line = line.replace(b'kid3</id>', b'kid3-qt</id>') \
                   .replace(b'kid3</binary>', b'kid3-qt</binary>') \
                   .replace(b'KDE</category>', b'Qt</category>') \
                   .replace(b'kid3.desktop', b'kid3-qt.desktop') \
                   .replace(b'>Kid3</name>', b'>Kid3-qt</name>')
        sys.stdout.buffer.write(line)


if __name__ == '__main__':
    fix_appdata()
