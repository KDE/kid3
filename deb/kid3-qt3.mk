#!/usr/bin/make -f

# These are used for cross-compiling and for saving the configure script
# from having to guess our platform (since we know it already)
DEB_HOST_GNU_TYPE   ?= $(shell dpkg-architecture -qDEB_HOST_GNU_TYPE)
DEB_BUILD_GNU_TYPE  ?= $(shell dpkg-architecture -qDEB_BUILD_GNU_TYPE)

ifneq (,$(filter noopt,$(DEB_BUILD_OPTIONS)))
CONFIGURE_DEBUG_OPTIONS = --enable-debug
endif

QMAKE = qmake-qt3
DEB_CONFIGURE_PREFIX = /usr

build: kid3-qt3.build-stamp

kid3-qt/configure: kid3-qt/configure.in
	cd kid3-qt && autoconf

kid3-qt3.build-stamp: kid3-qt/configure
	mkdir kid3-qt3; \
	cd kid3-qt3; \
	../kid3-qt/configure --host=$(DEB_HOST_GNU_TYPE) --build=$(DEB_BUILD_GNU_TYPE) \
	            --prefix=$(DEB_CONFIGURE_PREFIX) --with-qmake=$(QMAKE) $(CONFIGURE_DEBUG_OPTIONS); \
	cd ..; \
	$(MAKE) -C kid3-qt3

	touch kid3-qt3.build-stamp

clean: 
	[ ! -f kid3-qt3/Makefile ] || $(MAKE) -C kid3-qt3 distclean
	-rm -rf kid3-qt3.build-stamp kid3-qt3

install: build
	$(MAKE) -C kid3-qt3 install INSTALL_ROOT=$(CURDIR)/debian/kid3-qt

.PHONY: build clean install
