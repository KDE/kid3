#!/bin/bash
# You usually should not call this script, it documents the process.
# Use download-pos.sh instead, it will fetch the translations
# from l10n.kde.org.

BASEDIR="../src" # root of translatable sources
PROJECT="kid3" # project name
PROJECTVERSION="3.8.2" # project version
BUGADDR="http://sourceforge.net/p/kid3/bugs/" # MSGID-Bugs
WDIR=`pwd` # working dir

# Use extract-merge.sh if found in the path, its from
# svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/scripts
if test -f po/${PROJECT}_qt.pot; then
  echo "NOTE: Keeping po/${PROJECT}_qt.pot, delete it to extract messages"
elif hash extract-messages.sh 2>/dev/null; then
  echo "Using extract-messages.sh from l10n-kf5/scripts"
  cd ..
  mkdir -p po enpo
  extract-messages.sh
  cd -
  mkdir -p po
  mv ../po/* po
  rmdir ../po ../enpo
else
  echo "extract-messages.sh not found in path, falling back to custom extraction"

  echo "Preparing rc files"
  cd ${BASEDIR}
  # we use simple sorting to make sure the lines do not jump around too much from system to system
  find . -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > ${WDIR}/rcfiles.list
  xargs --arg-file=${WDIR}/rcfiles.list extractrc > ${WDIR}/rc.cpp
  cd ${WDIR}
  echo "Done preparing rc files"

  echo "Extracting messages"
  cd ${BASEDIR}
  # see above on sorting
  find . -name '*.cpp' -o -name '*.h' -o -name '*.c' -o -name '*.hpp' -o -name '*.qml' | sort > ${WDIR}/infiles.list
  echo "rc.cpp" >> ${WDIR}/infiles.list
  cd ${WDIR}
  mkdir -p po
  xgettext --from-code=UTF-8 -C --qt -ci18n -ktr:1,1t -ktr:1,2c,2t -ktr:1,1,2c,3t \
    -kQT_TRANSLATE_NOOP:1,1t -kQT_TRANSLATE_NOOP:2,2t \
    -kqsTr:1,1t -kqsTr:1,2c,2t -kqsTr:1,1,2c,3t \
    -kqsTrNoOp:1,1t -kqsTrNoOp:2,2t \
    --msgid-bugs-address="${BUGADDR}" \
    --package-name="${PROJECT^}" \
    --package-version="${PROJECTVERSION}" \
    --files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o po/${PROJECT}_qt.pot || { echo "error while calling xgettext. aborting."; exit 1; }
  echo "Done extracting messages"
  rm rcfiles.list
  rm infiles.list
  rm rc.cpp
fi

echo "Merging translations"
CATALOGS=`find po -name '*.po'`
for CAT in $CATALOGS; do
  echo $CAT
  msgmerge --quiet --update --backup=none $CAT po/${PROJECT}_qt.pot
done
echo "Done"
