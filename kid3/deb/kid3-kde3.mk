#!/usr/bin/make -f

# These are used for cross-compiling and for saving the configure script
# from having to guess our platform (since we know it already)
DEB_HOST_GNU_TYPE   ?= $(shell dpkg-architecture -qDEB_HOST_GNU_TYPE)
DEB_BUILD_GNU_TYPE  ?= $(shell dpkg-architecture -qDEB_BUILD_GNU_TYPE)

ifneq (,$(filter noopt,$(DEB_BUILD_OPTIONS)))
CFLAGS = -g3 -O0
CXXFLAGS = -g3 -O0
else
CFLAGS = -g -O2
CXXFLAGS = -g -O2
endif

pixmaps = $(CURDIR)/debian/kid3/usr/share/pixmaps
bindir = $(CURDIR)/debian/kid3/usr/bin
kdedocs = $(CURDIR)/debian/kid3/usr/share/doc/kde/HTML

build: kid3-kde3.build-stamp

-include debian/debiandirs

debian/debiandirs: admin/debianrules
	perl -w admin/debianrules echodirs > debian/debiandirs

configure: Makefile.cvs
	$(MAKE) -f Makefile.cvs

kid3-kde3.build-stamp: configure
	mkdir kid3-kde3; \
	cd kid3-kde3; \
	QTDIR=/usr/share/qt3 ../configure \
	                        --host=$(DEB_HOST_GNU_TYPE) \
	                        --build=$(DEB_BUILD_GNU_TYPE) \
	                        $(configkde); \
	cd ..; \
	$(MAKE) -C kid3-kde3

	touch kid3-kde3.build-stamp

clean: 
	[ ! -f kid3-kde3/Makefile ] || $(MAKE) -C kid3-kde3 distclean
	-rm -rf kid3-kde3.build-stamp kid3-kde3

install: build
	# Main install.
	$(MAKE) -C kid3-kde3 install DESTDIR=$(CURDIR)/debian/kid3

	# Make common links for docs.
	for i in `find $(kdedocs) -type d -name kid3 -not -regex .*/en/kid3`; do \
	  ln -f -n -s ../../en/common $$i/common; done

	perl -w admin/debianrules cleanup

.PHONY: build clean install
