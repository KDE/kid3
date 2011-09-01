#!/bin/sh
# Build Debian package.
mkdir debian
cp deb/* debian/
dpkg-buildpackage -rfakeroot
