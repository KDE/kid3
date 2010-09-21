# qmake project include file

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

contains($$list($$[QT_VERSION]), 4.*) {
  db2html.input = DOCBOOK_FILES
  QMAKE_EXTRA_COMPILERS += db2html
  DOCBOOK_FILES += index.docbook
} else {
  db2html.input = SOURCES
  QMAKE_EXTRA_UNIX_COMPILERS += db2html
  SOURCES += index.docbook
}

db2html.output = kid3_${QMAKE_TARGET}.html
isEmpty(CFG_DB2HTML) {
  exists(/usr/bin/xsltproc) {
    CFG_DB2HTML = xsltproc
  } else:exists(/usr/bin/xalan) {
    CFG_DB2HTML = xalan
  } else:exists(/usr/bin/jw) {
    CFG_DB2HTML = jw
  }
}
!isEmpty(CFG_PERL_CMD) {
  PERL = $$CFG_PERL_CMD
} else {
  PERL = perl
}
contains(CFG_DB2HTML, jw) {
  db2html.commands = $$PERL -n ../fixdocbook.pl <${QMAKE_FILE_NAME} >${QMAKE_FILE_BASE}.sgml; jw -f docbook -b html -u ${QMAKE_FILE_BASE}.sgml; $$PERL -n ../fixhtml.pl ${QMAKE_FILE_BASE}.html >${QMAKE_FILE_OUT}
} else {
  isEmpty(CFG_XSL_STYLESHEET) {
    exists(/usr/share/xml/docbook/stylesheet/nwalsh/html/docbook.xsl) {
      CFG_XSL_STYLESHEET = /usr/share/xml/docbook/stylesheet/nwalsh/html/docbook.xsl
    } else:exists(/usr/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl) {
      CFG_XSL_STYLESHEET = /usr/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl
    }
  }
  contains(CFG_DB2HTML, xsltproc) {
    db2html.commands = $$PERL -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | $$CFG_DB2HTML $$CFG_XSL_STYLESHEET - | $$PERL -n ../fixhtml.pl >${QMAKE_FILE_OUT}
  } else:contains(CFG_DB2HTML, xalan) {
    db2html.commands = $$PERL -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | $$CFG_DB2HTML -xsl $$CFG_XSL_STYLESHEET -out ${QMAKE_FILE_BASE}.html; $$PERL -n ../fixhtml.pl ${QMAKE_FILE_BASE}.html >${QMAKE_FILE_OUT}
  } else {
    db2html.commands = $$PERL -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | $$CFG_DB2HTML $$CFG_XSL_STYLESHEET - | $$PERL -n ../fixhtml.pl >${QMAKE_FILE_OUT}
  }
}

PRE_TARGETDEPS = kid3_${QMAKE_TARGET}.html
