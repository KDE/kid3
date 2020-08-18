#!/bin/bash
# Simplified variant of https://github.com/KDE/marble/tree/master/data/lang/download-pos.sh
#
# The translations are managed by KDE Localization.
# This script can be used to download them into this folder in order to build
# Kid3 with bundled translations.
# Translation status: https://l10n.kde.org/stats/gui/trunk-kf5/po/kid3_qt.po/
# To extract the translatable messages, proceed as described in
# https://techbase.kde.org/Development/Tutorials/Localization/i18n_Build_Systems
# Use --force-en as a parameter to force extraction of English plurals.

set -e

force_en=false
if test "$1" = "--force-en"; then
  force_en=true
  shift
fi

podir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/po
branch=${1:-trunk}
svn_path_prefix="svn://anonsvn.kde.org/home/kde/$branch/l10n-kf5"
svn_folder="messages/kid3"
mkdir -p "$podir"
svn -q export "$svn_path_prefix/templates/$svn_folder/kid3_qt.pot" "$podir/kid3_qt.pot"
echo "Downloaded po/kid3_qt.pot"
workdir="$(mktemp -d)"
pofile="$workdir/kid3_qt.po"
subdirs="$workdir/subdirs"
svn -q export "$svn_path_prefix/subdirs" $subdirs
for lang in $(cat $subdirs); do
  test "$lang" = "x-test" && continue
  svn -q export "$svn_path_prefix/$lang/$svn_folder/kid3_qt.po" $pofile >/dev/null 2>&1 || true
  if test -e $pofile; then
    target_dir="$podir/$lang"
    mkdir -p $target_dir
    mv -f $pofile $target_dir
    echo "Downloaded po/$lang"
  fi
done

if ! test -f po/en/kid3_qt.po || $force_en; then
  echo "Extracting po/en/kid3_qt.po"
  if hash extract-messages.sh 2>/dev/null; then
    cd ..
    mkdir -p po enpo
    extract-messages.sh
    cd -
    rm -rf ../po
    if test -f ../enpo/kid3_qt.po; then
      mkdir -p po/en
      mv ../enpo/kid3_qt.po po/en/
    else
      echo "ERROR: ../enpo/kid3_qt.po with English plurals not found."
      exit 1
    fi
    rmdir ../enpo
  else
    echo "ERROR: po/en/kid3_qt.po is missing and extract-messages.sh not found in PATH."
    echo "Get svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/scripts and add it to the PATH!"
    exit 1
  fi
else
  echo "Using existing po/en/kid3_qt.po"
fi
