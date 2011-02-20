#!/bin/sh

version=$(sed -n "s/Version:[ ]*\([0-9\.]*\).*/\1/ p" ../kid3.lsm)
mkdir kid3-macbin-${version}
mv Kid3.app kid3-macbin-${version}/
hdiutil create -srcfolder kid3-macbin-${version} kid3-macbin-${version}.dmg
mv kid3-macbin-${version}/Kid3.app .
rmdir kid3-macbin-${version}
