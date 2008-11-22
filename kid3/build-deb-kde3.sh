#!/bin/sh
# Build Debian package for KDE 3.
if mv -T deb debian; then
  DH_OPTIONS="-p kid3" dpkg-buildpackage -rfakeroot
  mv -T debian deb
fi
