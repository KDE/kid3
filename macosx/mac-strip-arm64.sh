#!/bin/bash
# Can be used to create an image with only amd64 binaries when using
# Qt6 with an OSX cross toolchain.
#
# Example usage:
# fatdmg=(kid3/*-Darwin.dmg)
# slimdmg=${fatdmg/-Darwin./-Darwin-amd64.}
# PATH=$PATH:/opt/osxcross/target/bin ../kid3/macosx/mac-strip-arm64.sh $fatdmg $slimdmg

SRC=${1?path to source dmg missing}
DST=${2?path to destination dmg missing}
if ! hash lipo 2>/dev/null; then
  echo "lipo not found"
  exit 1
fi
set -e
APPDIR=/tmp/inst
test -d "$APPDIR" && rm -rf "$APPDIR"
mkdir "$APPDIR"
# 7z x "$SRC" -o"$APPDIR" # does not preserve permissions and symlinks
DMGIMG=/tmp/dmg.img
dmg2img "$SRC" -o "$DMGIMG"
mkdir -p /tmp/mnt
archivemount "$DMGIMG" /tmp/mnt
#sudo mount -t iso9660 -o loop "$DMGIMG" /tmp/mnt
cp -a /tmp/mnt/* "$APPDIR"
fusermount -u /tmp/mnt
#sudo umount /mnt
rmdir /tmp/mnt
rm "$DMGIMG"

set +e
for f in $(find "$APPDIR" -type f); do
  ARCHS=$(lipo -archs $f 2>/dev/null)
  if test $? -eq 0; then
    if test -z "${ARCHS#*arm64*}"; then
      echo $f
      lipo "$f" -remove arm64 -output "$f"
    fi
  fi
done
set -e

genisoimage -V "Kid3" -D -R -apple -no-pad -o "$DST.uncompressed" "$APPDIR"
dmg dmg "$DST.uncompressed" "$DST"
rm "$DST.uncompressed"
rm -rf "$APPDIR"
