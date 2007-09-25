# qmake subdirectory project file

!include(../config.pri) {
  error("config.pri not found")
}

win32 {
  QMAKE_RUN_CC  = @echo
  QMAKE_RUN_CXX = @echo
  QMAKE_LINK    = @echo
}
!win32 {
  QMAKE_RUN_CC  = @echo > /dev/null
  QMAKE_RUN_CXX = @echo > /dev/null
  QMAKE_LINK    = @echo > /dev/null
}

QM_FILES = kid3_de.qm kid3_es.qm kid3_fr.qm kid3_ru.qm kid3_it.qm
QMAKE_CLEAN += $$QM_FILES

isEmpty(CFG_LRELEASE) {
  CFG_LRELEASE = lrelease
}
contains($$list($$[QT_VERSION]), 4.*) {
  ts2qm.input = TS_FILES
  ts2qm.commands = $$CFG_LRELEASE ${QMAKE_FILE_NAME} -qm ${QMAKE_FILE_OUT}
  QMAKE_EXTRA_COMPILERS += ts2qm
  TS_FILES += kid3_de.ts kid3_es.ts kid3_fr.ts kid3_ru.ts kid3_it.ts
} else {
  ts2qm.input = SOURCES
  ts2qm.commands = $$CFG_LRELEASE ${QMAKE_FILE_NAME} -qm ${QMAKE_FILE_OUT}
  QMAKE_EXTRA_UNIX_COMPILERS += ts2qm
  SOURCES += kid3_de.ts kid3_es.ts kid3_fr.ts kid3_ru.ts kid3_it.ts
}

ts2qm.output = ${QMAKE_FILE_BASE}.qm

PRE_TARGETDEPS = $$QM_FILES
isEmpty(CFG_TRANSLATIONSDIR) {
  unix:translation.path = /.
  win32:translation.path = \.
} else {
  translation.path = $$CFG_TRANSLATIONSDIR
}
unix:translation.extra = $(INSTALL_FILE) $$QM_FILES $(INSTALL_ROOT)$$CFG_TRANSLATIONSDIR
win32:translation.extra = for %%f in ($$QM_FILES) do $(INSTALL_FILE) %%f $(INSTALL_ROOT)$$CFG_TRANSLATIONSDIR

INSTALLS += translation
