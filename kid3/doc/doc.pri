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
unix {
  exists(/usr/bin/jw) {
    db2html.commands = perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} >${QMAKE_FILE_BASE}.sgml; jw -f docbook -b html -u ${QMAKE_FILE_BASE}.sgml; mv ${QMAKE_FILE_BASE}.html ${QMAKE_FILE_OUT}
  } else {
    DOCBOOK_XSL = docbook.xsl
    exists(/usr/share/xml/docbook/stylesheet/nwalsh/html/docbook.xsl) {
      DOCBOOK_XSL = /usr/share/xml/docbook/stylesheet/nwalsh/html/docbook.xsl
    } else:exists(/usr/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl) {
      DOCBOOK_XSL = /usr/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl
    }
    exists(/usr/bin/xsltproc) {
      db2html.commands = perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | xsltproc $$DOCBOOK_XSL - >${QMAKE_FILE_OUT}
    } else:exists(/usr/bin/xalan) {
      db2html.commands = perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | xalan -xsl $$DOCBOOK_XSL -out ${QMAKE_FILE_OUT}
    } else {
      db2html.commands = perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | xsltproc $$DOCBOOK_XSL - >${QMAKE_FILE_OUT}
    }
  }
}
win32:db2html.commands = %MSYSDIR%\bin\perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | %XSLTPROCDIR%\xsltproc --novalid --nonet %DOCBOOKDIR%\html\docbook.xsl - >${QMAKE_FILE_OUT}

PRE_TARGETDEPS = kid3_${QMAKE_TARGET}.html
