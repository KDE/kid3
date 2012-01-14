#!/bin/sh
# Build Debian package.
cp -R deb debian
rm -rf debian/source debian/watch
dpkg-buildpackage -rfakeroot
