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

LANGS = $$files(*.po)
LANGS ~= s/.po//g
QM_FILES = $$LANGS
QM_FILES ~= s/^(.*)$/kid3_\1.qm/g
TS_FILES = $$LANGS
TS_FILES ~= s/^(.*)$/kid3_\1.ts/g
contains($$list($$[QT_VERSION]), 4.*) {
  for(l, LANGS): exists($$[QT_INSTALL_DATA]/translations/qt_$${l}.qm): QT_QM_FILES += $${l}
  for(l, LANGS): exists($$[QT_INSTALL_DATA]/translations/qt_$${l}.ts): TS_FILES += $$[QT_INSTALL_DATA]/translations/qt_$${l}.ts
}
QMAKE_CLEAN += $$QM_FILES

isEmpty(CFG_LRELEASE) {
  CFG_LRELEASE = lrelease
}
contains($$list($$[QT_VERSION]), 4.*) {
  ts2qm.input = TS_FILES
  ts2qm.commands = $$CFG_LRELEASE ${QMAKE_FILE_NAME} -qm ${QMAKE_FILE_OUT}
  QMAKE_EXTRA_COMPILERS += ts2qm
} else {
  ts2qm.input = SOURCES
  ts2qm.commands = $$CFG_LRELEASE ${QMAKE_FILE_NAME} -qm ${QMAKE_FILE_OUT}
  QMAKE_EXTRA_UNIX_COMPILERS += ts2qm
  SOURCES += $$TS_FILES
}

ts2qm.output = ${QMAKE_FILE_BASE}.qm

PRE_TARGETDEPS = $$QM_FILES
isEmpty(CFG_TRANSLATIONSDIR) {
  unix:translation.path = /.
  win32:translation.path = \.
} else {
  translation.path = $$CFG_TRANSLATIONSDIR
}
contains($$list($$[QT_VERSION]), 4.*) {
  unix:translation.extra = $(INSTALL_FILE) $$QM_FILES $(INSTALL_ROOT)$$CFG_TRANSLATIONSDIR; for l in $$QT_QM_FILES; do $(INSTALL_FILE) $$[QT_INSTALL_DATA]/translations/qt_\$\${l}.qm $(INSTALL_ROOT)$$CFG_TRANSLATIONSDIR; done; true
  win32 {
    translation.extra = for %%f in ($$QM_FILES) do $(INSTALL_FILE) %%f $(INSTALL_ROOT)$$CFG_TRANSLATIONSDIR
    !isEmpty(QT_QM_FILES) {
      QT_INSTALL_DATA = $$[QT_INSTALL_DATA]
      translation.extra += & for %%l in ($$QT_QM_FILES) do $(INSTALL_FILE) $$replace(QT_INSTALL_DATA, /, \)\translations\qt_%%l.qm $(INSTALL_ROOT)$$CFG_TRANSLATIONSDIR
    }
  }
} else {
  unix:translation.extra = $(INSTALL_FILE) $$QM_FILES $(INSTALL_ROOT)$$CFG_TRANSLATIONSDIR; true
  win32:translation.extra = for %%f in ($$QM_FILES) do $(INSTALL_FILE) %%f $(INSTALL_ROOT)$$CFG_TRANSLATIONSDIR
}
INSTALLS += translation
