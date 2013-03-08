#!/bin/sh
# Build Debian package.
cp -R deb debian
rm -rf debian/source debian/watch
if test "$(lsb_release -sc)" = "squeeze"; then
  sed -i '/lib\(av\|chromaprint\)/ d' debian/control
  sed -i 's/-DWITH_MP4V2=/-DWITH_CHROMAPRINT=OFF -DWITH_MP4V2=/; s/dh_builddeb -- -Zxz/dh_builddeb/' debian/rules
fi
dpkg-buildpackage -rfakeroot
