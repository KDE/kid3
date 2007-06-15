# qmake subdirectory project file

!include(../../config.pri) {
  error("config.pri not found")
}
!include(../doc.pri) {
  error("doc.pri not found")
}

HTML_FILES = kid3_${QMAKE_TARGET}.html
QMAKE_CLEAN += $$HTML_FILES

isEmpty(CFG_DOCDIR) {
  unix:documentation.path = /.
  win32:documentation.path = \.
} else {
  documentation.path = $$CFG_DOCDIR
}
documentation.extra = $(INSTALL_FILE) $$HTML_FILES $(INSTALL_ROOT)$$CFG_DOCDIR

INSTALLS += documentation
