#!/bin/sh
# Build Debian package for KDE 4.
if mv -T deb debian; then
  DH_OPTIONS="-p kid3-kde4" dpkg-buildpackage -rfakeroot
  mv -T debian deb
fi
