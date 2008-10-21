#!/bin/sh
# Build Debian package for Qt.
if mv -T deb-qt debian; then
  dpkg-buildpackage -rfakeroot
  mv -T debian deb-qt
fi
