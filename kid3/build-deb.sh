#!/bin/sh
# Build Debian package.
mkdir debian
cp deb/* debian/
if test $(lsb_release -is) = "Ubuntu"; then
  if test $(lsb_release -cs) != "hardy"; then
    sedcmds="s/^# Ubuntu://; s/^# KDE4://"
  else
    sedcmds="s/^# Ubuntu://; s/^# KDE3://"
  fi
else
  if test $(lsb_release -cs) != "lenny"; then
    sedcmds="s/^# KDE4://"
  else
    sedcmds="s/^# KDE3://"
  fi
fi
sed -i "$sedcmds" debian/control
dpkg-buildpackage -rfakeroot
