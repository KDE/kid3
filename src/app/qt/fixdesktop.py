#!/usr/bin/env python

import fileinput
import sys
import platform


def fix_desktop():
    # Disable the global app menu on Ubuntu. Keyboard shortcuts are not
    # displayed, and they do not work unless their action is visible
    # in a toolbar.
    try:
        try:
            import distro
            is_ubuntu = distro.id() == 'ubuntu'
        except:
            is_ubuntu = platform.linux_distribution()[0].lower() == 'ubuntu'
    except:
        is_ubuntu = False
    if is_ubuntu:
        exec_start = 'Exec=env UBUNTU_MENUPROXY=0 kid3-qt'
    else:
        exec_start = 'Exec=kid3-qt'
    for line in fileinput.input():
        line = line\
          .replace('Name=Kid3', 'Name=Kid3-qt')\
          .replace('Exec=kid3', exec_start)\
          .replace('Icon=kid3', 'Icon=kid3-qt')\
          .replace('Categories=Qt;KDE', 'Categories=Qt')\
          .replace('StartupWMClass=kid3', 'StartupWMClass=kid3-qt')
        if not line.startswith('X-DocPath'):
            sys.stdout.write(line)

if __name__ == '__main__':
    fix_desktop()
