#!/bin/bash
# You usually should not call this script, it documents the process.
# Use download-pos.sh instead, it will fetch the translations
# and the docbooks which are considered to be ready from l10n.kde.org.
# To extract only a subset of the docbooks, pass their paths as parameters,
# e.g. ./extract-merge.sh po/pt/kid3.po.

set -e
_project="kid3"

if test -n "$1"; then
  _catalogs=$*
else
  _catalogs=`find po -name '*.po'`
fi

if ! test -f po/$_project.pot; then
  echo "Extracting messages"
  mkdir -p po
  xml2pot en/index.docbook >po/$_project.pot
else
  echo "NOTE: Keeping po/$_project.pot, delete it to extract messages"
fi

echo "Merging translations"
for _cat in $_catalogs; do
  echo $_cat
  msgmerge --quiet --update --backup=none $_cat po/${_project}.pot
done

echo "Generating docbooks"
for _cat in $_catalogs; do
  _lc=${_cat#po/}
  _lc=${_lc%/*}
  echo docs/$_lc/index.docbook
  mkdir -p docs/$_lc
  po2xml en/index.docbook $_cat >docs/$_lc/index.docbook
  _lang=$(sed -nE 's/"Language-Team:\s*(\S+)\s.*/\1/p' $_cat)
  test -n "$_lang" && sed -i "s/ENTITY % English/ENTITY % $_lang/" docs/$_lc/index.docbook
done

echo "Done"
