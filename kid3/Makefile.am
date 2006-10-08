
AUTOMAKE_OPTIONS = foreign 1.5
DISTCLEANFILES = inst-apps
MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files

include admin/deps.am

include admin/Doxyfile.am

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

EXTRA_DIST = admin debian AUTHORS COPYING ChangeLog INSTALL README make_qtonly.sh kid3.lsm kid3.spec

# generate precompiled header allsys.h.gch for GCC 3.4
pch:
	$(MAKE) -C kid3 pch

SUBDIRS=$(TOPSUBDIRS)
