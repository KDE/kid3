#!/usr/bin/make -f

pixmaps = $(CURDIR)/debian/kid3/usr/share/pixmaps
bindir = $(CURDIR)/debian/kid3/usr/bin
kdedocs = $(CURDIR)/debian/kid3/usr/share/doc/kde/HTML

build: kid3.build-stamp

-include debian/debiandirs

debian/debiandirs: admin/debianrules
	perl -w admin/debianrules echodirs > debian/debiandirs

configure: Makefile.cvs
	$(MAKE) -f Makefile.cvs

kid3.build-stamp: configure
	QTDIR=/usr/share/qt3 ./configure \
	                        --host=$(DEB_HOST_GNU_TYPE) \
	                        --build=$(DEB_BUILD_GNU_TYPE) \
	                        $(configkde)
	$(MAKE)

	touch kid3.build-stamp

clean: 
	[ ! -f Makefile ] || $(MAKE) distclean
	-rm -f kid3.build-stamp po/*.gmo

install: build
	# Main install.
	$(MAKE) install DESTDIR=$(CURDIR)/debian/kid3

	# Make common links for docs.
	for i in `find $(kdedocs) -type d -name kid3 -not -regex .*/en/kid3`; do \
	  ln -f -n -s ../../en/common $$i/common; done; done

	perl -w admin/debianrules cleanup

.PHONY: build clean install
