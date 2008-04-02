#!/bin/sh
# Build Debian package for KDE 4.
if mv -T deb-kde4 debian; then
  dpkg-buildpackage -rfakeroot
  mv -T debian deb-kde4
fi
