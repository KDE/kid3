#!/usr/bin/env python3

import fileinput
import sys


def fix_appdata():
    for line in fileinput.input():
        line = line.replace('kid3.desktop', 'kid3-qt.desktop')
        sys.stdout.write(line)

if __name__ == '__main__':
    fix_appdata()
