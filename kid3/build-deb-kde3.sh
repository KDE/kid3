#!/bin/sh
# Build Debian package for KDE 3 and Qt.
if mv -T deb-kde3 debian; then
  dpkg-buildpackage -rfakeroot
  mv -T debian deb-kde3
fi
