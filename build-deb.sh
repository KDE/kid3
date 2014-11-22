#!/bin/sh
# Build Debian package.
test -d debian && rm -rf debian
cp -R deb debian
# The PPA version (e.g. trusty1) can be given as a parameter to prepare
# a PPA upload. The source archive kid3_${version}.orig.tar.gz must be
# in the parent directory.
ppaversion=$1
if test -n "$ppaversion"; then
  distribution=${ppaversion%%[0-9]*}
  version=$(sed -e 's/^kid3 (\([0-9\.-]\+\).*$/\1/;q' debian/changelog)
  DEBEMAIL="Urs Fleisch <ufleisch@users.sourceforge.net>" \
  dch --newversion=${version}${ppaversion} --distribution=$distribution --urgency=low \
  "No-change backport to $distribution."
  sed -i -e 's/^Maintainer:.*$/Maintainer: Urs Fleisch <ufleisch@users.sourceforge.net>/;/^Uploaders:/,+1d' debian/control
  debuild -S -sa &&
  echo "PPA upload ready for $distribution. Use:" &&
  echo "cd ..; dput ppa:ufleisch/kid3 kid3_${version}${ppaversion}_source.changes"
else
  rm -rf debian/source debian/watch
  if test "$(lsb_release -sc)" = "squeeze"; then
    sed -i '/lib\(av\|chromaprint\)/ d' debian/control
    sed -i 's/-DWITH_MP4V2=/-DWITH_CHROMAPRINT=OFF -DWITH_MP4V2=/; s/dh_builddeb -- -Zxz/dh_builddeb/' debian/rules
  fi
  dpkg-buildpackage -rfakeroot
fi
