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
  isEmpty(CFG_DB2HTML) {
    exists(/usr/bin/jw) {
      CFG_DB2HTML = jw
    } else:exists(/usr/bin/xsltproc) {
      CFG_DB2HTML = xsltproc
    } else:exists(/usr/bin/xalan) {
      CFG_DB2HTML = xalan
    }
  }
  contains(CFG_DB2HTML, jw) {
    db2html.commands = perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} >${QMAKE_FILE_BASE}.sgml; jw -f docbook -b html -u ${QMAKE_FILE_BASE}.sgml; mv ${QMAKE_FILE_BASE}.html ${QMAKE_FILE_OUT}
  } else {
    isEmpty(CFG_XSL_STYLESHEET) {
      exists(/usr/share/xml/docbook/stylesheet/nwalsh/html/docbook.xsl) {
        CFG_XSL_STYLESHEET = /usr/share/xml/docbook/stylesheet/nwalsh/html/docbook.xsl
      } else:exists(/usr/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl) {
        CFG_XSL_STYLESHEET = /usr/share/apps/ksgmltools2/docbook/xsl/html/docbook.xsl
      }
    }
    contains(CFG_DB2HTML, xsltproc) {
      db2html.commands = perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | xsltproc $$CFG_XSL_STYLESHEET - >${QMAKE_FILE_OUT}
    } else:contains(CFG_DB2HTML, xalan) {
      db2html.commands = perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | xalan -xsl $$CFG_XSL_STYLESHEET -out ${QMAKE_FILE_OUT}
    } else {
      db2html.commands = perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | $$CFG_DB2HTML $$CFG_XSL_STYLESHEET - >${QMAKE_FILE_OUT}
    }
  }
}
win32:db2html.commands = %MSYSDIR%\bin\perl -n ../fixdocbook.pl <${QMAKE_FILE_NAME} | %XSLTPROCDIR%\xsltproc --novalid --nonet %DOCBOOKDIR%\html\docbook.xsl - >${QMAKE_FILE_OUT}

PRE_TARGETDEPS = kid3_${QMAKE_TARGET}.html
