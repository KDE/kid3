#!/bin/bash
# Build Debian package.
test -d debian && rm -rf debian
cp -R deb debian
# The PPA version (e.g. trusty1) can be given as a parameter to prepare
# a PPA upload. The source archive kid3_${version}.orig.tar.gz must be
# in the parent directory.
ppaversion=$1
if test -n "$ppaversion"; then
  distribution=${ppaversion%%[0-9]*}
  if which ubuntu-distro-info >/dev/null; then
    if distrib_nr=$(ubuntu-distro-info --series=$distribution -r 2>/dev/null); then
      distrib_nr=${distrib_nr/./}
      distrib_nr=${distrib_nr/ LTS/}
      distrib_id=Ubuntu
    elif distrib_nr=$(debian-distro-info --series=$distribution -r 2>/dev/null); then
      distrib_nr=${distrib_nr%.*}
      distrib_id=Debian
    else
      echo "Could not find release number for $distribution"
    fi
  else
    echo "distro-info is not installed"
  fi
else
  distribution=$(lsb_release -sc)
  distrib_nr=$(lsb_release -sr)
  distrib_nr=${distrib_nr/./}
  distrib_id=$(lsb_release -si)
fi

if (test $distrib_id = "Ubuntu" && test $distrib_nr -ge 1504) ||
   (test $distrib_id = "Debian" && test $distrib_nr -ge 9); then
  qtversion=5
else
  qtversion=4
fi

if test "$qtversion" = "5"; then
  sed -i -e '/# KDE 4 BEGIN/,/# KDE 4 END/{/# KDE/b;s/^/#/};/# KDE 5 BEGIN/,/# KDE 5 END/{/# KDE/b;s/^#//}' \
      debian/control debian/rules debian/kid3.install
fi
if test "$distribution" = "squeeze"; then
  sed -i '/lib\(av\|chromaprint\)/ d' debian/control
  sed -i 's/-DWITH_MP4V2=/-DWITH_CHROMAPRINT=OFF -DWITH_MP4V2=/; s/dh_builddeb -- -Zxz/dh_builddeb/' debian/rules
fi

if test -n "$ppaversion"; then
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
  debuild
fi
