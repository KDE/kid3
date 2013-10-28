#!/bin/bash

BASEDIR="../src" # root of translatable sources
PROJECT="kid3" # project name
PROJECTVERSION="3.0.1" # project version
BUGADDR="http://sourceforge.net/p/kid3/bugs/" # MSGID-Bugs
WDIR=`pwd` # working dir

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
find . -name '*.cpp' -o -name '*.h' -o -name '*.c' -o -name '*.hpp' | sort > ${WDIR}/infiles.list
echo "rc.cpp" >> ${WDIR}/infiles.list
cd ${WDIR}
xgettext --from-code=UTF-8 -C --qt --no-location -ci18n -ktr:1,1t -ktr:1,2c,2t -ktr:1,1,2c,3t \
  -kQT_TRANSLATE_NOOP:1,1t -kQT_TRANSLATE_NOOP:2,2t \
  --msgid-bugs-address="${BUGADDR}" \
  --package-name="${PROJECT^}" \
  --package-version="${PROJECTVERSION}" \
  --files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o ${PROJECT}.pot || { echo "error while calling xgettext. aborting."; exit 1; }
echo "Done extracting messages"


echo "Merging translations"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
  echo $cat
  msgmerge --quiet --update --backup=none $cat ${PROJECT}.pot
  sed -i "/#, qt-format/ d; /#, kde-format/ d; /^#~ msg/ d" $cat
done
echo "Done merging translations"


echo "Cleaning up"
cd ${WDIR}
rm rcfiles.list
rm infiles.list
rm rc.cpp
echo "Done"
