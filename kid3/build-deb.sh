#!/bin/sh
# Build Debian package for KDE 3.
if mv -T deb debian; then
  dpkg-buildpackage -rfakeroot -tc
  mv -T debian deb
fi
