#!/bin/bash
# Variant of translations/download-pos.sh
#
# The docs are managed by KDE Localization.
# This script can be used to download them into this folder in order to build
# Kid3 with bundled handbooks.
# Translation status: https://l10n.kde.org/stats/gui/trunk-kf5/po/kid3_qt.po/
# To extract the translatable messages, use xml2pot en/index.docbook >kid3.pot
# On Ubuntu, this tool can be found in package poxml.

set -e
docdir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
podir=$docdir/po
docsdir=$docdir/docs
branch=${1:-trunk}
svn_path_prefix="svn://anonsvn.kde.org/home/kde/$branch/l10n-kf5"
svn_folder="docmessages/extragear-multimedia"
svn_docs_folder="docs/extragear-multimedia/kid3"
mkdir -p "$podir"
svn -q export "$svn_path_prefix/templates/$svn_folder/kid3.pot" "$podir/kid3.pot"
echo "Downloaded po/kid3.pot"
workdir="$(mktemp -d)"
pofile="$workdir/kid3.po"
dbfile="$workdir/index.docbook"
subdirs="$workdir/subdirs"
svn -q export "$svn_path_prefix/subdirs" $subdirs
for lang in $(cat $subdirs); do
  test "$lang" = "x-test" && continue
  svn -q export "$svn_path_prefix/$lang/$svn_folder/kid3.po" $pofile >/dev/null 2>&1 || true
  if test -e $pofile; then
    target_dir="$podir/$lang"
    mkdir -p $target_dir
    mv -f $pofile $target_dir
    echo "Downloaded po/$lang"
  fi
  svn -q export "$svn_path_prefix/$lang/$svn_docs_folder/index.docbook" $dbfile >/dev/null 2>&1 || true
  if test -e $dbfile; then
    target_dir="$docsdir/$lang"
    mkdir -p $target_dir
    mv -f $dbfile $target_dir
    echo "Downloaded docs/$lang"
  fi
done
